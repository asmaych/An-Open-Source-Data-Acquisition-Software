#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/datetime.h>
#include <fstream>
#include <sstream>
#include <map>
#include "ProjectPanel.h"
#include "ExportManager.h"
#include "CalibrationRestorer.h"
#include "Events.h"
#include "HandshakeDialog.h"
#include "SensorConfigDialog.h"
#include "data/GraphWindow.h"
#include "data/LiveDataWindow.h"
#include "data/DataTableWindow.h"
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "sensor/SensorManager.h"
#include "sensor/Interpolator.h"
#include "serial/SerialComm.h"
#include "SensorSelectionDialog.h"
#include "data/Run.h"
#include "controllers/SessionController.h"
#include "MainFrame.h"
#include "CalibrationPoint.h"
#include "HardwareConfirmDialog.h"


// ==================== CONSTRUCTION / DESTRUCTION ============================

/**
 * @brief initialises all wxWidgets controls, creates owned subsystems, and binds event handlers
 */
ProjectPanel::ProjectPanel(wxWindow* parent, const wxString& title, DatabaseManager* db)
    	     : wxPanel(parent, wxID_ANY)
{
    	m_projectName = title;
    	m_DB = db;

    	//one vertical sizer owns the entire panel layout
    	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    	SetSizer(sizer);

    	//create SerialComm instance for this project (not connected yet)
	//each project gets its own serial connection so multiple projects can be open simultaneously on different ports
    	m_serial = std::make_unique<SerialComm>();

    	//sensorManager holds a reference to m_sensors so it sees additions and removals immediately without needing to be notified 
	//separately
    	m_sensorManager = std::make_unique<SensorManager>(m_sensors, m_serial.get());

    	//ExportManager holds a reference to m_sensors for column headers, it is constructed here so it is always ready even before 
	//a run starts
    	m_exporter = std::make_unique<ExportManager>(this, m_sensors);

    	//bind incoming events from the background serial thread and from the collect-now table window
    	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
    	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);
    	Bind(wxEVT_COLLECT_NOW_POINT, &ProjectPanel::onCollectNowGraphPoint, this);

    	//main splitter has graph on top, bottom area below
    	m_splitter = new wxSplitterWindow(this, wxID_ANY);
    	m_splitter -> SetSashGravity(0.6);
    	m_splitter -> SetMinimumPaneSize(120);

    	//bottom splitter has live data window and collect-now table side by side
    	m_bottom_splitter = new wxSplitterWindow(m_splitter, wxID_ANY);
    	m_bottom_splitter -> SetSashGravity(0.5);
    	m_bottom_splitter -> SetMinimumPaneSize(120);

    	//child windows, all start hidden until a run begins or data is loaded
    	m_graphWindow = new GraphWindow(m_splitter);
    	m_liveWindow  = new LiveDataWindow(m_bottom_splitter);;
    	m_tableWindow = nullptr;

    	m_splitter -> Hide();
    	m_bottom_splitter -> Hide();
    	m_graphWindow -> Hide();
    	m_liveWindow -> Hide();

	m_graphRefreshTimer.Bind(wxEVT_TIMER, [this](wxTimerEvent&){
    		if(m_isRunning && m_currentRun && m_graphWindow) graphRun(m_currentRun);});

    	sizer -> Add(m_splitter, 1, wxEXPAND);
    	Layout();
}


/**
 * @brief stops any active run, resets the microcontroller, and cleans up owned resources
 */
ProjectPanel::~ProjectPanel()
{
    	stopRun();

    	//send a reset command to the microcontroller so it stops streaming and can be reused cleanly by the next session or project
    	try{
        	m_serial -> reset();
    	}
    	catch(const std::exception& e){
        	std::cout << "No microcontroller was reset — none was connected.";
    	}
}


// ===================== SERIAL CONNECTION =============================

/**
 * @brief opens the serial port selection and handshake dialog
 */
void ProjectPanel::openConnectDialog()
{
    	HandshakeDialog dlg(this, "Connect", m_serial.get());
    	dlg.ShowModal();
}


/**
 * @brief called when the handshake thread reports success or failure
 *
 * on success, registers any preloaded sensors with the microcontroller, creates the SessionController, restores the sample rate,
 * and starts the polling loop
 */
void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
    	bool success = evt.GetPayload<bool>();
    	if(!success){
        	wxLogStatus("Handshake failed!");
        	return;
    	}

    	handshakeComplete = true;
    	wxLogStatus("Handshake successful!");

    	/* register all sensors with the microcontroller. This covers two cases:
    		1. Project was loaded from DB before connecting: sensors exist in m_sensors but hardware was never told about them
    		2. Sensors were added without a connection (template setup): same situation, hardware needs to be informed now
	  We call m_serial -> addSensor() directly here rather than through SensorManager to avoid pushing duplicates into m_sensors
	*/
	if(!m_sensors.empty()){
        	std::cout << "Registering " << m_sensors.size() << " sensors with ESP32\n";
        	for(const auto& sensor : m_sensors){
 			m_serial -> addSensor(sensor -> getName(), sensor -> getPin());
    			std::cout << "  -> sent: " << sensor -> getName() << " pin " << sensor -> getPin() << "\n";
		}
    	}

 	//create the session controller now that a live connection exists, the controller owns the background polling thread that fires
    	//wxEVT_SERIAL_UPDATE events into this panel
    	m_controller = std::make_unique<SessionController>(m_serial.get(), &m_runs, this, &m_sensors);

    	//restore the saved sample rate, this sends the rate to the microcontroller and must happen after the handshake
    	if(projecthasbeenloaded)
        	adjustSampleRate(m_hz);

    	m_controller -> start();
}


// ===================== RUN CONTROL ==================================

/**
 * @brief toggles between starting a new run and stopping the current one
 *
 * requires an active hardware connection. Sends start/stop commands to the esp32 before delegating to startRun() or stopRun()
 */
void ProjectPanel::toggleStartStop()
{
    	if(!handshakeComplete){
        	wxMessageBox("Please connect first.", "Warning");
        	return;
    	}

    	if(!m_isRunning){
        	//tell the microcontroller to begin streaming sensor data
        	if(m_serial)
			m_serial -> writeString("start");
        	startRun();
        	wxLogStatus("Run Started");
    	}
    	else{
        	//tell the microcontroller to stop streaming
        	if(m_serial)
			m_serial -> writeString("stop");
        	stopRun();
        	wxLogStatus("Run stopped");
    	}
}


/**
 * @brief creates a new Run object, resets the collect-now table, shows the live windows, and creates the DB run row
 */
void ProjectPanel::startRun()
{
    	graphVisible = true;
    	liveVisible = true;
    	collectNowVisible = false;

    	//destroy any existing collect-now table so the new run starts clean.
	//the splitter must be unsplit before resetting the table window to avoid a dangling child window inside the splitter
    	if(m_tableWindow){
        	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
            		m_bottom_splitter -> Unsplit();
        	if(m_bottom_splitter)
            		m_bottom_splitter -> Initialize(nullptr);
        	m_tableWindow -> Destroy();
    		m_tableWindow = nullptr;
    	}

    	updateLayout();

    	//create a new run with an incrementing run number so historical runs are distinguishable in the graph and live window tabs
    	m_currentRun = std::make_shared<Run>(m_runs.size() + 1);
    	m_runs.push_back(m_currentRun);

    	//store absolute OS time so incoming frames can be expressed as "seconds since this run started"
    	m_runStartTime = wxGetUTCTimeMillis().ToDouble() / 1000.0;
    	m_isRunning = true;

    	m_liveWindow -> startNewRun(m_currentRun, m_sensors);

    	//create the run row in the DB immediately so frame writes have a valid run_id from the very first frame
    	if(m_saveProjectToDB && m_projectId >= 0){
        	m_currentRunId = m_DB -> createRun(m_projectId);
        	std::cout << "Run created in DB: runId=" << m_currentRunId << " for projectId=" << m_projectId << "\n";
    	}

	m_graphRefreshTimer.Start(100); // refresh graph at 10fps max

    	Layout();
}


/**
 * @brief stops the active run, saves UI state to the DB, and clears the running flag
 */
void ProjectPanel::stopRun()
{
    	if(!m_isRunning)
		return;

    	m_liveWindow -> stopRun();
    	m_isRunning = false;

    	//persist which panels were visible so they are restored on next load
    	if(m_saveProjectToDB && m_projectId >= 0)
        	m_DB -> saveUIState(m_projectId, graphVisible, liveVisible, collectNowVisible);

	m_graphRefreshTimer.Stop();
}


// ===================== DATA FLOW ===============================

/**
 * @brief receives a parsed data frame from the serial thread, stores it in the active run, saves it to the DB, and forwards it to the
 * live display.
 */
void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
    	if(!m_isRunning || !m_currentRun)
		return;

    	auto values = evt.GetPayload<std::vector<double>>();
    	if(values.empty())
		return;

    	//convert absolute OS time to seconds since this run started
    	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;
    	double t = now - m_runStartTime;

    	m_currentRun -> addFrame(t, values);

    	//write the frame to the DB inside the run-level transaction
    	//frame_values references the frame row so the frame must be inserted first to get a valid frame_id
    	if(m_saveProjectToDB && m_currentRunId >= 0) {
        	int frameId = m_DB -> createFrame(m_currentRunId, t);
        	if(frameId >= 0){
            		for(size_t i = 0; i < values.size(); ++i){
                		int sensorId = m_DB -> getSensorID(m_sensors[i] -> getName());
                		m_DB -> saveFrameValue(frameId, sensorId, values[i]);
            		}
        	}
    	}

    	m_liveWindow -> addFrame(t, values);
}


/**
 * @brief called by the SessionController on every polling tick to push raw sensor readings into the active run and live display
 *
 * this path is used when SessionController drives the frame loop directly rather than relying on the serial thread event
 */
void ProjectPanel::onNewDataFrame()
{
    	if(!m_isRunning || !m_currentRun)
		return;

    	//collect the latest reading from each sensor
    	std::vector<double> values;
    	values.reserve(m_sensors.size());
    	for(auto& sensor : m_sensors)
        	values.push_back(sensor -> getReading());

    	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;
    	double t = now - m_runStartTime;

    	m_currentRun -> addFrame(t, values);

    	//save to DB
    	if(m_saveProjectToDB && m_currentRunId >= 0){
        	int frameId = m_DB -> createFrame(m_currentRunId, t);
        	if(frameId >= 0){
            		for(size_t i = 0; i < values.size(); ++i){
                		int sensorId = m_DB -> getSensorID(m_sensors[i] -> getName());
                		m_DB -> saveFrameValue(frameId, sensorId, values[i]);
            		}
        	}
    	}

    	if(m_liveWindow)
		m_liveWindow -> addFrame(t, values);
    	//if(m_graphWindow)
	//	graphRun(m_currentRun);
}


// ====================== LAYOUT =================================

/**
 * @brief reorganizing the splitter hierarchy to match the current visibility flags (graphVisible, liveVisible, collectNowVisible)
 *
 * All eight combinations of the three flags are handled. The splitters are unsplit before reconfiguring to avoid wxWidgets assertion 
 * failures from double-splitting
 */
void ProjectPanel::updateLayout()
{
    	if(!m_splitter)
		return;

    	//unsplit all splits and hide everything first to get a clean slate
    	if(m_splitter -> IsSplit())
		 m_splitter -> Unsplit();
    	if(m_bottom_splitter -> IsSplit())
		m_bottom_splitter -> Unsplit();

    	m_splitter -> Hide();
    	m_graphWindow -> Hide();
    	m_liveWindow -> Hide();
    	if(m_tableWindow)
		m_tableWindow -> Hide();

    	//nothing to show then return with an empty panel
    	if(!graphVisible && !liveVisible && !collectNowVisible){
        	Layout();
        	return;
    	}

    	m_splitter -> Show();

    	//Graph + Live (default active-run view)
    	if(graphVisible && liveVisible && !collectNowVisible){
        	if(m_bottom_splitter -> IsSplit())
			m_bottom_splitter -> Unsplit();

		m_graphWindow -> Show();
        	m_liveWindow -> Show();
        	m_bottom_splitter -> Initialize(m_liveWindow);
        	m_splitter -> Initialize(m_graphWindow);
        	m_splitter -> SplitHorizontally(m_graphWindow, m_bottom_splitter, GetSize().GetHeight() * 0.6);
    	}

    	//graph only
    	else if(graphVisible && !liveVisible && !collectNowVisible){
        	m_graphWindow -> Show();
        	m_splitter -> Initialize(m_graphWindow);
    	}

    	//live only
    	else if(liveVisible && !graphVisible && !collectNowVisible){
        	m_liveWindow -> Show();
        	m_bottom_splitter -> Initialize(m_liveWindow);
        	m_splitter -> Initialize(m_bottom_splitter);
    	}

    	//collect-now only
    	else if(!graphVisible && !liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit()) 
			m_bottom_splitter -> Unsplit();
        	if(m_tableWindow){
           		m_tableWindow -> Show();
            		m_bottom_splitter -> Initialize(m_tableWindow);
            		m_splitter -> Initialize(m_bottom_splitter);
        	}
    	}

    	//graph + collect-now
    	else if(graphVisible && !liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit()) 
			m_bottom_splitter -> Unsplit();
        	if(m_tableWindow){
            		m_tableWindow -> Show();
            		m_graphWindow -> Show();
            		m_bottom_splitter -> Initialize(m_tableWindow);
            		m_splitter -> Initialize(m_graphWindow);
            		m_splitter -> SplitHorizontally(m_graphWindow, m_bottom_splitter, GetSize().GetHeight() * 0.6);
        	}
    	}

    	//live + collect-now
    	else if(!graphVisible && liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit())
			m_bottom_splitter -> Unsplit();
        	m_liveWindow -> Show();
        	if(m_tableWindow){
            		m_tableWindow -> Show();
            		m_bottom_splitter -> Initialize(m_liveWindow);
            		m_bottom_splitter -> SplitVertically(m_liveWindow, m_tableWindow);
            		m_splitter -> Initialize(m_bottom_splitter);
        	}
    	}

    	//all three
    	else if(graphVisible && liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit()) 
			m_bottom_splitter -> Unsplit();
        	m_graphWindow -> Show();
        	m_liveWindow -> Show();
        	if(m_tableWindow){
            		m_tableWindow -> Show();
            		//graph on top, live and collect-now side by side below
            		m_bottom_splitter -> Initialize(m_liveWindow);
            		m_bottom_splitter -> SplitVertically(m_liveWindow, m_tableWindow);
        	}
        	m_splitter -> Initialize(m_graphWindow);
        	m_splitter -> SplitHorizontally(m_graphWindow, m_bottom_splitter, GetSize().GetHeight() * 0.6);
    	}

    	Layout();
}


// ======================= COLLECT ON DEMAND =============================

/**
 * @brief snapshots the most recent frame from the active run into the collect-now table and shows it
 *
 * creates the DataTableWindow on first call. Updates the layout. Does nothing if the run has no data yet.
 */
void ProjectPanel::collectCurrentValues()
{
    	collectNowVisible = true;

    	if(!m_currentRun)
		return;

    	const auto& frames = m_currentRun -> getFrames();
    	const auto& times  = m_currentRun -> getTimes();

    	//verify at least one non-empty frame exists before continuing
    	bool hasData = false;
    	for(const auto& frame : frames)
        	if(!frame.empty()) { 
			hasData = true; break;
		}

    	if(!hasData){
        	wxMessageBox("No data available to collect!", "Error", wxOK | wxICON_ERROR);
        	return;
    	}

    	//create the table window on first collect, one column per sensor plus a leading Time column
    	if(!m_tableWindow){
        	std::vector<std::shared_ptr<DataSession>> sessions;
        	sessions.push_back(std::make_shared<DataSession>("Time"));
        	for(auto& s : m_sensors)
        	    	sessions.push_back(std::make_shared<DataSession>(s -> getName()));

        	m_tableWindow = new DataTableWindow(m_bottom_splitter, sessions, m_currentRun);
    	}

    	updateLayout();
}


/**
 * @brief receives a completed collect-now row from DataTableWindow, saves it to the DB, and draws the demand circle/point on the graph
 *
 * The row vector format is: [time, sensor0_value, sensor1_value, ...]
 * Ownership of the heap-allocated row vector is transferred here and deleted at the end of the function
 */
void ProjectPanel::onCollectNowGraphPoint(wxCommandEvent& evt)
{
    	if(!m_graphWindow)
		return;

    	auto row = static_cast<std::vector<double>*>(evt.GetClientData());
    	if(!row || row -> empty())
		return;

    	double time = (*row)[0];

    	//save each sensor's value to the DB if project saving is enabled, a valid run is currently active & each collected point is 
	//saved per sensor, meaning row[1] -> sensor 0, row [2] -> sensor 1
    	if(m_saveProjectToDB && m_currentRunId >= 0){
        	for(size_t i = 1; i < row -> size(); ++i){
            		if(i - 1 < m_sensors.size()){
                		int sensorId = m_DB -> getSensorID(m_sensors[i-1] -> getName());
                		m_DB -> saveCollectPoint(m_currentRunId, sensorId, time, (*row)[i]);
            		}
        	}
        std::cout << "Collect point saved at t=" << time << "\n";
    	}

    	//draw a highlighted demand circle on the graph for each sensor.
    	//curve IDs follow the format "run{N}_sensor{i}" which matches the IDs used by graphRun() so circles appear on the right curves
    	for(size_t i = 1; i < row -> size(); ++i){
        	if(i - 1 < m_sensors.size()){
            		size_t sensorIndex = i - 1;
            		size_t runNumber   = m_currentRun -> getRunNumber();
            		std::string curveId = "run" + std::to_string(runNumber) + "_sensor" + std::to_string(sensorIndex);
            		m_graphWindow -> addDemandPoint(curveId, time, (*row)[i]);
        	}
    	}

	//to avoid memory leaks we gotta delet the row vector manually
    	delete row;
}


// ===================== RESET ================================

/**
 * @brief clears all runs, live data, graph curves, and the collect-now table. Resets all state flags to their initial values
 */
void ProjectPanel::resetSessionData()
{
    	if(!m_currentRun && m_runs.empty())
		return;

    	if(m_liveWindow)
		m_liveWindow -> clearAll();
    	if(m_graphWindow)
		m_graphWindow -> clear();

    	//unsplit before destroying the table window to avoid a wxWidget splitter problems
    	if(m_tableWindow){
        	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
            		m_bottom_splitter -> Unsplit();
        	if(m_bottom_splitter)
            		m_bottom_splitter -> Initialize(nullptr);
        	m_tableWindow -> Destroy();
    		m_tableWindow = nullptr;
    	}

    	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
        	m_bottom_splitter -> Unsplit();
    	if(m_splitter && m_splitter -> IsSplit())
        	m_splitter -> Unsplit();

    	graphVisible = false;
    	liveVisible = false;
    	collectNowVisible = false;

    	m_runs.clear();
    	m_currentRun.reset();
    	m_isRunning = false;

    	updateLayout();
    	Layout();
 	wxLogStatus("All data cleared.");
}

// ====================== GRAPHING ==================================

/**
 * @brief graphs the current run, or the collect-now table's associated run if a table exists
 */
void ProjectPanel::graphSelectedSensor(wxCommandEvent& evt)
{
    	if(!m_currentRun)
		return;

    	bool hasData = false;
    	for(const auto& frame : m_currentRun -> getFrames())
        	if(!frame.empty()){
			hasData = true;
			break;
		}

    	if(!hasData){
        	wxMessageBox("No data available to graph!", "Error", wxOK | wxICON_ERROR);
        	return;
    	}

    	if(!m_graphWindow){
        	m_graphWindow = new GraphWindow(this);
        	m_graphWindow -> setTheme(m_currentTheme);
    	}

    	m_graphWindow -> Show();

    	if(m_tableWindow)
		graphTable(m_tableWindow);
    	if(m_currentRun)
		graphRun(m_currentRun);
}


/**
 * @brief graphs the run associated with a collect-now table
 *
 * The collect-now table is a view over a run, so graphing the table is same to graphing the run that produced it
 *
 * @param table  Pointer to the DataTableWindow whose run should be graphed
 */
void ProjectPanel::graphTable(DataTableWindow* table)
{
    	if(!table)
		return;

    	auto run = table ->getAssociatedRun();
    	if(!run){
        	wxMessageBox("This table is not associated with any run.", "Error");
        	return;
    	}

    	graphRun(run);
}


/**
 * @brief adds one curve per sensor from the given run to the graph window
 *
 * Each curve is identified by a unique ID in the format "run{N}_sensor{i}" so multiple runs and sensors are kept separate
 *
 * @param run  Shared pointer to the run whose frames should be graphed
 */
void ProjectPanel::graphRun(const std::shared_ptr<Run>& run)
{
    	auto& times  = run -> getTimes();
    	auto& frames = run -> getFrames();

    	if(frames.empty() || frames[0].empty())
		return;

    	size_t sensorCount = frames[0].size();
    	size_t runNumber = run -> getRunNumber();

    	for(size_t sensor = 0; sensor < sensorCount; ++sensor){
        	std::vector<double> y;
        	for(auto& frame : frames)
            		y.push_back(frame[sensor]);

        	std::string name = m_sensors[sensor] -> getName();
        	std::string id = "run" + std::to_string(runNumber) + "_sensor" + std::to_string(sensor);

        	m_graphWindow -> addCurve(times, y, name, runNumber, id);
    	}
}


// ================== EXPORT =============================

/**
 * @brief export calls ExportManager, passes the current run, all runs, the collect-now table, and the graph window
 */
void ProjectPanel::exportSessions(wxCommandEvent& evt)
{
   	m_exporter -> exportSessions(m_currentRun, m_runs, m_tableWindow, m_graphWindow);
}


/**
 * @brief forwards askSaveFile to ExportManager's static helper
 */
wxString ProjectPanel::askSaveFile(wxWindow* parent)
{
    	return ExportManager::askSaveFile(parent);
}


// ================== SENSOR CONFIGURATION ==========================

/**
 * @brief opens the SensorConfigDialog for adding, removing, calibrating, and loading sensors, works without a hardware connection
 */
void ProjectPanel::onSensors()
{
    	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_DB, m_sensors);
    	dlg.ShowModal();
}


// =================== LOAD PROJECT FROM DATABASE ================================

/**
 * @brief fully restores a saved project from the db
 *
 * Steps performed in order:
 *   1. Full in-memory reset (stops any active run, clears all windows)
 *   2. Load sample rate
 *   3. Load sensors → show HardwareConfirmDialog for pin verification
 *   4. Register sensors with hardware if already connected
 *   5. Load and apply UI state (which panels were visible)
 *   6. Load all runs → replay frames into graph and live window
 *   7. Restore calibration via CalibrationRestorer
 *   8. Restore collect-now table and demand circles if visible
 *   9. CallAfter(updateLayout) to ensure splitter sizes correctly
 */
void ProjectPanel::loadProjectFromDatabase()
{
    	projecthasbeenloaded = true;

    	if(!m_DB || m_projectId < 0)
		return;

    	// 1. Full in-memory reset
    	//always start from a clean slate so reloading a project produces exactly the same result as loading it for the first time
    	if(m_isRunning)
		stopRun();

    	if(m_graphWindow)
		m_graphWindow -> clear();
    	if(m_liveWindow)
		m_liveWindow -> clearAll();

    	if(m_tableWindow){
        	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
            		m_bottom_splitter -> Unsplit();
        	m_tableWindow -> Destroy();
    		m_tableWindow = nullptr;
    	}

    	m_runs.clear();
    	m_currentRun.reset();
    	m_currentRunId = -1;
    	m_isRunning = false;
    	graphVisible = false;
    	liveVisible = false;
    	collectNowVisible = false;

    	// 2. Load sample rate
    	m_DB -> loadProjectSampleRate(m_projectId, m_hz);

    	//3. Load sensors
    	m_sensorManager -> clearSensors();
    	m_sensors.clear();

    	std::vector<std::pair<std::string, int>> sensors;
    	m_DB -> loadProjectSensors(m_projectId, sensors);

    	std::cout << "Sensors retrieved from db: " << sensors.size() << "\n";

    	if(sensors.empty())
		return;

    	for(auto& [name, pin] : sensors)
        	m_sensors.push_back(std::make_unique<Sensor>(name, pin));

    	//show hardware confirmation dialog every time a project loads
	//the user can verify or update pin assignments before proceeding
	//if they cancel, abort the load and leave the panel clean
	HardwareConfirmDialog hwDlg(nullptr, sensors, m_projectId, m_DB);
    	if(hwDlg.ShowModal() == wxID_CANCEL){
        	m_sensorManager -> clearSensors();
        	m_sensors.clear();
        	return;
    	}

    	//rebuild m_sensors with the (possibly updated) pins from the dialog
    	m_sensors.clear();
    	for(auto& [name, pin] : sensors)
        	m_sensors.push_back(std::make_unique<Sensor>(name, pin));

    	std::cout << "SensorManager now has: " << sensors.size() << " sensors\n";

    	//4. Register with hardware if already connected
    	if(m_serial && m_serial -> handshakeresult)
        	for(const auto& sensor : m_sensors)
            		m_serial -> addSensor(sensor -> getName(), sensor -> getPin());

    	//5. Load UI state
    	bool graph, live, collect;
    	m_DB -> loadUIState(m_projectId, graph, live, collect);
    	graphVisible = graph;
    	liveVisible = live;
    	collectNowVisible = collect;

    	//6. Load runs
    	std::vector<std::pair<int,int>> runRows;
    	m_DB -> loadProjectRuns(m_projectId, runRows);
    	std::cout << "Runs retrieved from db: " << runRows.size() << "\n";

    	for(auto& [dbRunId, runNumber] : runRows){
        	auto run = std::make_shared<Run>(runNumber);

        	std::vector<std::pair<double, std::vector<double>>> frames;
        	m_DB -> loadRunFrames(dbRunId, m_sensors, frames);

        	for(auto& [t, values] : frames)
            	run -> addFrame(t, values);

        	m_runs.push_back(run);

        	if(!run -> getFrames().empty()){
            		graphRun(run);
            		m_liveWindow -> addHistoricalRun(run, m_sensors);
        	}

        	std::cout << " Run " << runNumber << " loaded with " << run -> getFrames().size() << " frames\n";
    	}

    	//7. Restore calibration
    	//calls CalibrationRestorer which follows the priority logic implemented (project calibration wins over global template)
    	CalibrationRestorer restorer(m_DB, m_sensorManager.get(), m_projectId);
    	restorer.restore(m_sensors);

    	//8. Restore collect-now table
	// m_tableWindow must exist before updateLayout() is called, otherwise the splitter has nothing to display
    	if(collectNowVisible && !m_runs.empty()){
        	std::vector<std::shared_ptr<DataSession>> sessions;
        	sessions.push_back(std::make_shared<DataSession>("Time"));
        	for(auto& sensor : m_sensors)
            	sessions.push_back(std::make_shared<DataSession>(sensor -> getName()));

        	//associate the table with the last loaded run
        	m_tableWindow = new DataTableWindow(m_bottom_splitter, sessions, m_runs.back());

        	if(!runRows.empty()){
            		int lastDbRunId = runRows.back().first;

            		std::vector<std::tuple<double,int,double>> collectPts;
            		m_DB -> loadCollectPoints(lastDbRunId, collectPts);

            		std::cout << "Collect points loaded: " << collectPts.size() << "\n";

            		//group by timestamp, one collect event produces one row per sensor, so rows are keyed on time and values are
            		//placed at the correct sensor column index
            		std::map<double, std::vector<double>> rowMap;

            		for(auto& [t, sensorId, value] : collectPts)
                		if(rowMap.find(t) == rowMap.end())
                    			rowMap[t] = std::vector<double>(m_sensors.size(), 0.0);

            		for(auto& [t, sensorId, value] : collectPts)
                		for(size_t i = 0; i < m_sensors.size(); ++i)
                    			if(m_DB -> getSensorID(m_sensors[i] -> getName()) == sensorId){
                        			rowMap[t][i] = value;
                        			break;
                    			}

 			//restore table rows
    			for(auto& [t, sensorValues] : rowMap){
                		std::vector<double> row;
                		row.push_back(t);
                		row.insert(row.end(), sensorValues.begin(), sensorValues.end()); m_tableWindow -> appendRow(row);
            		}

            		//restore demand circles on the graph using the same curve ID format used during the original session
            		for(auto& [t, sensorValues] : rowMap)
                		for(size_t i = 0; i < sensorValues.size(); ++i){
                    			std::string curveId = "run" + std::to_string(m_runs.back() -> getRunNumber()) + "_sensor" + std::to_string(i);
                    			m_graphWindow -> addDemandPoint(curveId, t, sensorValues[i]);
                		}

            		std::cout << "Collect points restored: " << rowMap.size() << " rows\n";
        		}
    	}

   	//9. Update layout
    	//CallAfter waits until the ui is ready so the splitter has the right size before splitting
	CallAfter([this](){ updateLayout(); });
}


// ====================== THEME =============================

/**
 * @brief propagates a light/dark theme change to all child windows
 *
 * @param theme  The new theme to apply
 */
void ProjectPanel::applyTheme(Theme theme)
{
    	m_currentTheme = theme;

    	wxColour bg = (theme == Theme::Dark) ? wxColour(30, 30, 30) : *wxWHITE;
    	wxColour fg = (theme == Theme::Dark) ? wxColour(220, 220, 220) : *wxBLACK;

    	SetBackgroundColour(bg);
    	SetForegroundColour(fg);

    	if(m_liveWindow)
		m_liveWindow -> applyTheme(theme);
    	if(m_tableWindow)
		m_tableWindow -> applyTheme(theme);
    	if(m_graphWindow)
		m_graphWindow -> setTheme(theme);

    	Refresh();
}


// ==================== SAMPLE RATE ================================

/* \brief	This function handles the event that the button for adding a new sensor is pressed in the dialog interface.
 *
 * 		The only thing necessary is to stack allocate an instance of the AddSensorDialog, passing along the pointer to the
 * 		sensorManager owned by the parent Project. Any modification to the vector of sensors is made by SensorManager, 
 *              and not this dialog  or any other dialog.
 *
 * @param rate  Wanted sample rate in Hz
 */
void ProjectPanel::adjustSampleRate(const int rate)
{
    	if(!m_serial->handshakeresult)
	{
        	wxMessageBox("Please connect with a microcontroller first!");
        	return;
    	}

    	//convert Hz to millisecond delay (1000ms / rate)
    	m_sampleRate = 1000 / rate;

    	m_serial->adjustPollingRate(m_sampleRate);
    	m_controller->setInterval(m_sampleRate);
    	m_DB->saveProjectSampleRate(m_projectId, m_sampleRate);
}


// ====================== GETTERS ===================================

/** @brief returns a raw pointer to the live data window. */
LiveDataWindow* ProjectPanel::getLiveWindow()
{
    return m_liveWindow;
}

/** @brief returns a raw pointer to the collect-now table window. */
DataTableWindow* ProjectPanel::getTableWindow()
{
    return m_tableWindow;
}

/** @brief returns a raw pointer to the graph window. */
GraphWindow* ProjectPanel::getGraphWindow()
{
    return m_graphWindow;
}

/** @brief returns the currently active run, or nullptr if none. */
std::shared_ptr<Run> ProjectPanel::getCurrentRun()
{
    return m_currentRun;
}

/** @brief returns true if an experiment is currently running. */
bool ProjectPanel::isRunning() const
{
	return m_isRunning;
}

/** @brief returns true if the hardware handshake has completed. */
bool ProjectPanel::isConnected() const
{
	return handshakeComplete;
}

/** @brief returns a const reference to the project's sensor vector. */
const std::vector<std::unique_ptr<Sensor>>& ProjectPanel::getSensors() const
{
    return m_sensors;
}

/** @brief returns true if this project is being saved to the DB. */
bool ProjectPanel::shouldSaveProject() const 
{
	return m_saveProjectToDB;
}

/** @brief returns the DB ID of this project, or -1 if unsaved. */
int ProjectPanel::getProjectID() const
{ 
	return m_projectId;
}

/** @brief Returns a raw pointer to the SensorManager. */
SensorManager* ProjectPanel::getSensorManager() const
{
    return m_sensorManager.get();
}

/** @brief returns the current sample rate in milliseconds. */
float ProjectPanel::getSampleRate() const
{
	return m_sampleRate;
}

/** @brief returns elapsed seconds since the current run started, if no run is active return 0. */
int ProjectPanel::getElapsedSeconds() const
{
    	if(!m_isRunning || m_runStartTime <= 0.0)
        	return 0;

    	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;
    	return static_cast<int>(now - m_runStartTime);
}
