#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/datetime.h>
#include <fstream>
#include <sstream>
#include "ProjectPanel.h"
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

ProjectPanel::ProjectPanel(wxWindow* parent, const wxString& title, DatabaseManager* db)
	: wxPanel(parent, wxID_ANY)
{
	m_projectName = title;
	m_DB = db;

	//one sizer for the entire panel
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// ================ serial + sensor ===========
	//Create SerialComm instance for this project (not connected yet)
	m_serial = std::make_unique<SerialComm>();

	//create a SensorManager class and point m_sensorManager to it
	//Note that the sensorManager is given the address of the class
	//member m_sensors vector in order to modify it, along with the
	//project-specific instance of SerialComm to get readings.
	m_sensorManager = std::make_unique<SensorManager>(m_sensors, m_serial.get());

	//Bind events
	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);
	Bind(wxEVT_COLLECT_NOW_POINT, &ProjectPanel::onCollectNowGraphPoint, this);

	//create splitter (graph on top, bottom area below)
	m_splitter = std::make_unique<wxSplitterWindow>(this, wxID_ANY);
	m_splitter -> SetSashGravity(0.6);
	m_splitter -> SetMinimumPaneSize(120);

	//bottom splitter (live/table)
	m_bottom_splitter = std::make_unique<wxSplitterWindow>(m_splitter.get(), wxID_ANY);
	m_bottom_splitter -> SetSashGravity(0.5);
	m_bottom_splitter -> SetMinimumPaneSize(120);

	//child windows
	m_graphWindow = std::make_unique<GraphWindow>(m_splitter.get());
	m_liveWindow = std::make_unique<LiveDataWindow>(m_bottom_splitter.get());
	m_tableWindow = nullptr;

	//start hidden
	m_splitter -> Hide();
	m_bottom_splitter -> Hide();
	m_graphWindow -> Hide();
	m_liveWindow -> Hide();

	//put main splitter in sizer
	sizer -> Add(m_splitter.get(), 1, wxEXPAND);

	Layout(); //enforce initial layout

}


ProjectPanel::~ProjectPanel()
{
	/* \brief	This destructor is explicitly implemented so that:
	 *
	 * 		- Threads created during runtime get shut down properly
	 *
	 * 		- m_serial SerialComm object can reset the connected
	 * 		microcontroller configuration so that it can be used
	 * 		again without problems.
	 */

	stopRun();

	//reset the connected arduino if it exists using the SerialComm instance
	try
	{
		m_serial->reset();
	}
	catch (const std::exception& e)
	{
		wxLogStatus("No microcontroller was reset, because none was connected.");
	}
}


//opens the dialog that lets the user pick a serial port and perform the handshake with the microprocessor
void ProjectPanel::openConnectDialog()
{
	HandshakeDialog dlg(this, "Connect", m_serial.get());
	dlg.ShowModal();
}


//called when the handshake dialog finishes, to check whether the connection succeeded or not
void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
	
	bool success = evt.GetPayload<bool>();
	if(!success){
 		wxLogStatus("Handshake failed!");
		return;
	}

	handshakeComplete = true;

	wxLogStatus("Handshake successful!");

	//if project_sensors were loaded from DB before connecting, then we need to  register them with the microcontroller now
	if(!m_sensors.empty()){
        	std::cout << "Registering " << m_sensors.size() << " preloaded sensors with ESP32\n";
        	for(const auto& sensor : m_sensors){
            		m_serial->addSensor(sensor -> getName(), sensor -> getPin());
            		std::cout << "  -> sent: " << sensor -> getName() << " pin " << sensor -> getPin() << "\n";
        	}
    	}

	//Create the session controller
	m_controller = std::make_unique<SessionController>(m_serial.get(), &m_runs, this, &m_sensors);

	if (projecthasbeenloaded)
		adjustSampleRate(m_hz);

	//start polling immediately
	m_controller -> start();
}


// ========================== RUN CONTROL ===============================

//called when start/stop button is pressed to toggle between starting a new run and stopping the current one
void ProjectPanel::toggleStartStop()
{
	if(!handshakeComplete){
		wxMessageBox("Please connect first.", "Warning");
		return;
	}

	if(!m_isRunning){
		// ================= START RUN ==================
		// tell the microprocessor to start streaming
		if(m_serial){
			m_serial -> writeString("start");
		}

		//start the run
		startRun();

		wxLogStatus("Run Started");
	}

	else{
		// ================= STOP RUN ======================
		//tell the microprocessor to stop streaming
		if(m_serial){
			m_serial -> writeString("stop");
		}

		//stop the run
		stopRun();

		wxLogStatus("Run stopped");
	}
}


//starts a new continuous acquisition run taht stores timestamps and frmaes (vector of sensor values)
void ProjectPanel::startRun()
{
	graphVisible = true;
	liveVisible = true;
	collectNowVisible = false;

	//reset the table for collect now if it does exist
	//destroy table completely
        if(m_tableWindow){
                //if bottom splitter is using the table, unsplit first
                if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
                        m_bottom_splitter -> Unsplit();

                //if bottom splitter was intialized with table only
                if(m_bottom_splitter)
                        m_bottom_splitter -> Initialize(nullptr);

                m_tableWindow.reset();
        }

	updateLayout();

	//create a new run with an incrementing run number
	m_currentRun = std::make_shared<Run>(m_runs.size() + 1);
	m_runs.push_back(m_currentRun);

	//store absolute OS time so we can convert to "run time"
	m_runStartTime = wxGetUTCTimeMillis().ToDouble() / 1000.0;
	m_isRunning = true;

	//start live display for this run
	m_liveWindow -> startNewRun(m_currentRun, m_sensors);

	//create run in db
	if(m_saveProjectToDB && m_projectId >= 0){
		m_currentRunId = m_DB -> createRun(m_projectId);
		std::cout << "Run created in DB: runId=" << m_currentRunId << " for projectId=" << m_projectId <<"\n";
	}

	std::cout << "Sensors available for experiment: " << m_sensors.size()  << "\n";

	for (auto& s : m_sensors) {
    		std::cout << " -> " << s->getName() << " pin " << s->getPin() << std::endl;
	}

	Layout(); //force layout update
}


//stops the current run and the data remains stored in m_run
void ProjectPanel::stopRun()
{
	if(!m_isRunning)
		return;

	//stop live display from appending new data
	m_liveWindow -> stopRun();

	m_isRunning = false;

	//save UI state
	if(m_saveProjectToDB && m_projectId >= 0){
		m_DB -> saveUIState(m_projectId, graphVisible, liveVisible, collectNowVisible);
	}
}

// ========================= DATA FLOW ==========================

//called whenever the microcontroller sends a new frame of data
void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
	//ignore data if no run is active
	if(!m_isRunning || !m_currentRun)
		return;

	auto values = evt.GetPayload<std::vector<double>>(); //where each element is a sensor value

	if(values.empty())
		return;

	//current OS time in seconds
	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;

	//convert OS time into "time since this run started"
	double t = now - m_runStartTime;

	//store permanently in the run object
	m_currentRun -> addFrame(t, values);

	//save frames to db
	if(m_saveProjectToDB && m_currentRunId >= 0){
		//create the frame first
	 	int frameId = m_DB->createFrame(m_currentRunId, t);

    		if(frameId >= 0){
			for(size_t i = 0; i < values.size(); i++){
		        	//use sensor ID instead of name
            			int sensorId = m_DB -> getSensorID(m_sensors[i] -> getName());

            			m_DB -> saveFrameValue(frameId, sensorId, values[i]);
        		}

    		}
	}

	//display it
	m_liveWindow -> addFrame(t, values);
}

// ===================<==== LAYOUT =====================
void ProjectPanel::updateLayout()
{
   	if (!m_splitter)
        	return;

	if(m_splitter -> IsSplit())
		m_splitter -> Unsplit();

	if(m_bottom_splitter -> IsSplit())
		m_bottom_splitter -> Unsplit();

    	// reset everything
    	m_splitter -> Hide();
    	m_graphWindow -> Hide();
    	m_liveWindow -> Hide();

    	if (m_tableWindow)
        	m_tableWindow -> Hide();

    	// ========= NOTHING =========
    	if (!graphVisible && !liveVisible && !collectNowVisible){
		Layout();
		return;
    	}

    	m_splitter -> Show();

        // ========= GRAPH + LIVE (default run view) =========
	if (graphVisible && liveVisible && !collectNowVisible){
	   	if(m_bottom_splitter -> IsSplit())
			m_bottom_splitter -> Unsplit();
		m_graphWindow -> Show();
        	m_liveWindow -> Show();

		m_bottom_splitter -> Initialize(m_liveWindow.get());
		m_splitter -> Initialize(m_graphWindow.get());
        	m_splitter -> SplitHorizontally(m_graphWindow.get(), m_bottom_splitter.get(), GetSize().GetHeight() * 0.6);
       }

    	// ========= GRAPH ONLY =========
    	else if (graphVisible && !liveVisible && !collectNowVisible){
		m_graphWindow -> Show();

		m_splitter -> Initialize(m_graphWindow.get());
  	}

    	// ========= LIVE ONLY =========
    	else if (liveVisible && !graphVisible && !collectNowVisible){
        	m_liveWindow -> Show();

		m_bottom_splitter -> Initialize(m_liveWindow.get());
		m_splitter -> Initialize(m_bottom_splitter.get());
	}

    	// ========= COLLECT NOW ONLY =========
    	else if (!graphVisible && !liveVisible && collectNowVisible){
		if(m_bottom_splitter -> IsSplit())
			m_bottom_splitter -> Unsplit();
		if(m_tableWindow){
			m_tableWindow -> Show();
			m_bottom_splitter -> Initialize(m_tableWindow.get());
			m_splitter -> Initialize(m_bottom_splitter.get());
  		}
	}

	 // ========= GRAPH + COLLECT =========
    	else if (graphVisible && !liveVisible && collectNowVisible){
 		if(m_bottom_splitter -> IsSplit())
                        m_bottom_splitter -> Unsplit();

		if(m_tableWindow){
			m_tableWindow -> Show();
			m_graphWindow -> Show();

			m_bottom_splitter -> Initialize(m_tableWindow.get());
			m_splitter -> Initialize(m_graphWindow.get());
			m_splitter->SplitHorizontally(m_graphWindow.get(), m_bottom_splitter.get(),GetSize().GetHeight() * 0.6);
        	}
	}

    	// ========= LIVE + COLLECT =========
    	else if (!graphVisible && liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit())
                        m_bottom_splitter -> Unsplit();

		m_liveWindow -> Show();
		if(m_tableWindow){
		   	m_tableWindow -> Show();
			m_bottom_splitter -> Initialize(m_liveWindow.get());
	        	m_bottom_splitter -> SplitVertically(m_liveWindow.get(), m_tableWindow.get());
			m_splitter -> Initialize(m_bottom_splitter.get());
		}
	}

    	// ========= ALL THREE =========
    	else if (graphVisible && liveVisible && collectNowVisible){
        	if(m_bottom_splitter -> IsSplit())
                        m_bottom_splitter -> Unsplit();

		m_graphWindow -> Show();
        	m_liveWindow -> Show();
		if(m_tableWindow){
	        	m_tableWindow -> Show();

			//graph on top while table and collectNow are sidebyside
			m_bottom_splitter -> Initialize(m_liveWindow.get());
     			m_bottom_splitter -> SplitVertically(m_liveWindow.get(), m_tableWindow.get());
		}

		m_splitter -> Initialize(m_graphWindow.get());
		m_splitter -> SplitHorizontally(m_graphWindow.get(), m_bottom_splitter.get(), GetSize().GetHeight() * 0.6);
	}

	//recalculate layout after all visibility/split changes
	Layout();
}

// ======================= COLLECT ON DEMAND =====================

//takes the most recent frame from the active run and plug it into a table 
void ProjectPanel::collectCurrentValues()
{
	collectNowVisible = true;

	//check if there is an active run
	if(!m_currentRun){
		//wxMessageBox("No active run to collect from!", "Warning");
		return;
	}

	//no values received
	bool hasData = false;
        const auto& frames = m_currentRun -> getFrames();
	const auto& times = m_currentRun -> getTimes();
        for (const auto& frame : frames){
                if(!frame.empty()) {
                        hasData = true;
                        break;
                }
        }

        if(!hasData){
                wxMessageBox("No data available to collect!", "Error", wxOK | wxICON_ERROR);
                return;
        }

	//get the very last frame and timestamp recorded
	const std::vector<double>& lastValues = frames.back();
	double lastT = times.back();

	if(!m_tableWindow){
		std::vector<std::shared_ptr<DataSession>> sessions;
                sessions.push_back(std::make_shared<DataSession>("Time")); //first column
                for(auto& s : m_sensors)
                        sessions.push_back(std::make_shared<DataSession>(s -> getName()));

                m_tableWindow = std::make_unique<DataTableWindow>(m_bottom_splitter.get(), sessions, m_currentRun);
	}

	updateLayout();
}


// =============== Graph Collected on demand Points ==============

//draws a point (highlighted circle) in each time we collect on demand a value
void ProjectPanel::onCollectNowGraphPoint(wxCommandEvent& evt)
{
	//if no graph/curve
	if(!m_graphWindow)
		return;

	//get the row of data sent by DataTableWindow (time § sensor values)
	//evt.GetClientData stores a pointer to std::vector<double> of format [time, sensor1_val, sensor2_val..]
	auto row = static_cast<std::vector<double>*>(evt.GetClientData());

	//if times & frames are empty
	if(!row || row -> empty())
		return;

	//first element is always time (x-axis)
	double time = (*row)[0];

	// ============ DB ==============
	/* we only save if:
		- project saving is enabled
		- a valid run is currently active
		- each collected point is saved per sensor meaning: row[1] -> sensor 0, row [2] -> sensor 1
	*/
    	if(m_saveProjectToDB && m_currentRunId >= 0){
        	for(size_t i = 1; i < row->size(); ++i){
            		//ensure we don't go out of bounds
			if(i - 1 < m_sensors.size()){
				//get sensor name from current project sensor list then retrieve its global DB id
                		int sensorId = m_DB -> getSensorID(m_sensors[i-1]->getName());
				//save collected point into db to the right run, sensor, using the right timestamp & collected value
                		m_DB -> saveCollectPoint(m_currentRunId, sensorId, time, (*row)[i]);
            		}
        	}
        	std::cout << "Collect point saved at t = " << time << "\n";
    	}

    	//draw the highlighted circle on the graph
    	for(size_t i = 1; i < row->size(); ++i){
        	double value = (*row)[i];

        	if(i - 1 < m_sensors.size()){
            		size_t sensorIndex = i - 1;
			//each curve is identified by runX_sensorY which ensures multiple runs are seperated and multiple sensors are tracked per run
            		size_t runNumber = m_currentRun -> getRunNumber();
            		std::string curveId = "run" + std::to_string(runNumber) + "_sensor" + std::to_string(sensorIndex);
            		m_graphWindow -> addDemandPoint(curveId, time, value);
        	}
    	}

	//to avoid memory leaks we gotta delet the row vector manually
    	delete row;
}


// ============================ RESET ============================

//clears all runs and all live data, everything starts from scratch
void ProjectPanel::resetSessionData()
{
	if(!m_currentRun && m_runs.empty()){
               	return;
        }

	//clear live and graph
	if(m_liveWindow)
		m_liveWindow -> clearAll();

    	if(m_graphWindow)
		m_graphWindow -> clear();

	//destroy table completely
	if(m_tableWindow){
		//if bottom splitter is using the table, unsplit first
		if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
			m_bottom_splitter -> Unsplit();

		//if bottom splitter was intialized with table only
		if(m_bottom_splitter)
			m_bottom_splitter -> Initialize(nullptr);

                m_tableWindow.reset();
        }

	//reset splitters
	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
		m_bottom_splitter -> Unsplit();

	if (m_splitter && m_splitter -> IsSplit())
        	m_splitter -> Unsplit();

	// =========== DB CLEANUP ==========
	//delete this session's runs from DB
/*    	if(m_saveProjectToDB && m_projectId >= 0){
        	for(auto& run : m_runs){
            		//only delete runs that have a DB ID, loaded historical runs are in m_runs but we don't track their DB IDs here,
            		// so we can only delete the current session's run so for now, delete the current run ID if it exists.
        	}
        	//delete the current run's frames and the run itself
        	if(m_currentRunId >= 0){
            		m_DB -> deleteRun(m_currentRunId);
            		m_currentRunId = -1;
        	}
    	}
*/
	// Reset state flags
    	graphVisible = false;
    	liveVisible = false;
    	collectNowVisible = false;

    	//reset runs
	m_runs.clear();
    	m_currentRun.reset();
    	m_isRunning = false;

	updateLayout();

    	Layout();

	wxLogStatus("all data cleared");
}

void ProjectPanel::resetTableWindow()
{
	m_tableWindow.reset();
}

// ============================= GRAPH ===========================
void ProjectPanel::graphSelectedSensor(wxCommandEvent& evt)
{
	if(!m_currentRun){
                //wxMessageBox("No active run to graph!", "Error", wxOK | wxICON_ERROR);
                return;
        }

        bool hasData = false;
        const auto& frames = m_currentRun -> getFrames();
        for (const auto& frame : frames){
                if(!frame.empty()) {
                        hasData = true;
                        break;
                }
        }

        if(!hasData){
                wxMessageBox("No data available to graph!", "Error", wxOK | wxICON_ERROR);
                return;
        }

	//if the window doesn't exist create it, and if its hidden, unhidden it
	if(!m_graphWindow){
		m_graphWindow = std::make_unique<GraphWindow>(this);
		m_graphWindow -> setTheme(m_currentTheme);
	}

	m_graphWindow -> Show();

	// Decide source - is it collect on demand or active run
    	if (m_tableWindow) {
        	//graph collect on demand table
        	graphTable(m_tableWindow.get());
	}

	//otherwise, graph the current run
	if (m_currentRun){
		graphRun(m_currentRun);
	}
}


// ======================= GRAPH COLLECT NOW ======================
void ProjectPanel::graphTable(DataTableWindow* table)
{
	if(!table)
		return;

	auto run = table -> getAssociatedRun();

	if(!run){
		wxMessageBox("This table is not associated with any run.", "Error");
		return;
	}

	//a collect on demand table is just a view of a run here
	//so we grave the run that was used to produce the table
	graphRun(run);
}



// ========================== GRAPH RUN ============================
void ProjectPanel::graphRun(const std::shared_ptr<Run>& run)
{
	auto& times = run -> getTimes();  //vector<double> of times
	auto& frames = run -> getFrames(); // (vector<vector<double>>: rows = time, columns = sensors)

	if(frames.empty() || frames[0].empty())
		return;

	size_t sensorCount = frames[0].size();
	size_t runNumber = run -> getRunNumber();

	// ================== LOOP OVER SENSORS ===============
	//each sensor gets its own curve
	for(size_t sensor = 0; sensor < sensorCount; ++sensor) {
		std::vector<double> y;

		//extract this sensor's values from each frame
		for(auto& frame: frames){
			y.push_back(frame[sensor]);
		}

		//Assign the name of the sensor
		std::string name = m_sensors[sensor] -> getName();

		//unique id = runNumber + sensorIndex
		std::string id = "run" + std::to_string(run -> getRunNumber()) + "_sensor" + std::to_string(sensor);

		//add the curve to the graph window
		m_graphWindow -> addCurve(times, y, name, runNumber, id);
	}
}


// ======================= GETTERS =========================
LiveDataWindow* ProjectPanel::getLiveWindow()
{
	return m_liveWindow.get();
}

DataTableWindow* ProjectPanel::getTableWindow()
{
	return m_tableWindow.get();
}

GraphWindow* ProjectPanel::getGraphWindow()
{
	return m_graphWindow.get();
}

std::shared_ptr<Run> ProjectPanel::getCurrentRun()
{
	return m_currentRun;
}

bool ProjectPanel::isRunning() const
{
	return m_isRunning;
}

bool ProjectPanel::isConnected() const
{
	return handshakeComplete;
}

const std::vector<std::unique_ptr<Sensor>>& ProjectPanel::getSensors() const
{
	return m_sensors;
}

bool ProjectPanel::shouldSaveProject() const
{
	return m_saveProjectToDB;
}

int ProjectPanel::getProjectID() const
{
	return m_projectId;
}

// ======================== EXPORT ==========================
void ProjectPanel::exportSessions(wxCommandEvent& evt)
{
	//allow export if we have either a current run or historical runs, cause when a project is loaded, m_currentRun is null but m_runs has data
    	bool hasCurrent = (m_currentRun != nullptr);
    	bool hasAny = (!m_runs.empty());

    	if(!hasCurrent && !hasAny)
        	return;

    	//for export purposes, use m_currentRun if it exists, otherwise fall back to the last loaded historical run
    	std::shared_ptr<Run> runToExport = m_currentRun ? m_currentRun : m_runs.back();

    	//check that the run actually contains data
    	bool hasData = false;
    	for(const auto& frame : runToExport -> getFrames()){
        	if(!frame.empty()){
        	    	hasData = true;
            		break;
        	}
    	}

	//abort export if run has no data
	if(!hasData){
		wxMessageBox("No data available to export!", "Error", wxOK | wxICON_ERROR);
		return;
	}

	// ============= export options dialog =============
	//dialog allowing the user to choose what to export
	wxDialog dlg(this, wxID_ANY, "Export options", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	//export option checkboxes
	wxCheckBox* cbContinuous = new wxCheckBox(&dlg, wxID_ANY, "Collect Continuous");
	wxCheckBox* cbCollectNow = new wxCheckBox(&dlg, wxID_ANY, "Collect Now");
	wxCheckBox* cbGraph = new wxCheckBox(&dlg, wxID_ANY, "Graph");

	//sensible defaults
        cbContinuous -> SetValue(m_currentRun != nullptr);
        cbCollectNow -> SetValue(m_tableWindow != nullptr);
        cbGraph -> SetValue(m_graphWindow != nullptr);

        sizer -> Add(cbContinuous, 0, wxALL, 5);
        sizer -> Add(cbCollectNow, 0, wxALL, 5);
        sizer -> Add(cbGraph, 0, wxALL, 5);

        // Buttons
        wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
        btnSizer -> AddButton(new wxButton(&dlg, wxID_OK));
        btnSizer -> AddButton(new wxButton(&dlg, wxID_CANCEL));
        btnSizer -> Realize();

        sizer -> Add(btnSizer, 0, wxEXPAND | wxALL, 10);

        dlg.SetSizerAndFit(sizer);

        if (dlg.ShowModal() != wxID_OK)
                return;

	//handle whether collect now table wasn't created
	if(cbCollectNow -> IsChecked() && !m_tableWindow){
		wxMessageBox("Collect now table wasn't created!", "Export Error", wxOK | wxICON_WARNING);
		return;
	}

	// ============ continue with the export ===========
	wxString path = askSaveFile(this);
	if(path.IsEmpty())
		return;

	//remove extension so we can reuse base name
	path = path.BeforeLast('.');

	// =============== perform exports ===============
	bool didExport = false;

	//continuous run export to csv
	if(cbContinuous -> IsChecked() && runToExport)
	{
		exportRun(runToExport, path + "_collect_continuous.csv");
		didExport = true;
	}

	//collect now table export
	if (cbCollectNow->IsChecked() && m_tableWindow)
	{
	       //I did not handle check if collect now has data or only time cause it shouldn't already work without data 
	        exportTable(m_tableWindow.get(), path + "_collect_now.csv");
               	didExport = true;
	}

	//graph export to png
	if (cbGraph->IsChecked() && m_graphWindow)
        {
                m_graphWindow->exportImage(path + "_graph.png");
                didExport = true;
        }

	if (!didExport)
        {
                wxMessageBox("Nothing was selected to export.", "Export", wxOK | wxICON_WARNING);
                return;
        }

	wxMessageBox("Export Complete!", "Info", wxOK | wxICON_INFORMATION);
}


// ===================== HELPERS TO EXPORT ==================
wxString ProjectPanel::askSaveFile(wxWindow* parent)
{
	/*we use a file dialog to ask the user where to save the CSV file
		this : parent window
		"save csv" : our dialog title
	        "" : default directory (empty means current working directory)
		"CSV files (*.csv) | * .csv": filter to show only csv files (no other files)
		wxFD_SAVE | ...: flags to save nd warn if overwriting an existing file
	*/
	wxFileDialog dlg(parent, "Export data", "", "", "CSV files (*.csv) |*.csv|TextFiles(*.txt)|*.txt",
			 wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	//show the dialog and check if the user presses cancel
	if(dlg.ShowModal() == wxID_CANCEL)
		return "";

	wxString path = dlg.GetPath();

	if(!path.EndsWith(".csv") && !path.EndsWith(".txt"))
		path += ".csv";

	return path;
}

void ProjectPanel::exportTable(DataTableWindow* table, const wxString& path)
{
	if(!table || m_sensors.empty())
		return;

	std::ofstream file(path.ToStdString());

	if(!file.is_open())
		return;

	const auto& times = table -> getTimes();
	const auto& values = table -> getValues();

	if(times.empty())
		return;

	//header
	file << "Time";
	for(size_t sensor = 0; sensor < values.size(); ++sensor)
		file << "," << m_sensors[sensor] -> getName();
	file << "\n";

	//rows
	for(size_t row = 0; row < times.size(); ++row){
		file << times[row];
		for(size_t sensor = 0; sensor < values.size(); ++sensor){
			file << "," << values[sensor][row];
		}
		file << "\n";
	}
	file.close();
}

void ProjectPanel::exportGraph(GraphWindow* graph, const wxString& path)
{
	if(!graph)
		return;

	const auto& curves = graph -> getCurves(); //vector <Curve>
	if(curves.empty())
		return;

	std::ofstream file(path.ToStdString(), std::ios::app); //append to table if exists

	if(!file.is_open())
		return;

	//write header
	file << "Time";
	for(const auto& curve : curves){
		file << "," << curve.label;
	}
	file << "\n";

	//find row count
	size_t points = curves[0].x.size(); //time base
	for(const auto& curve : curves){
		points = std::min(points, curve.x.size());
	}

	//write rows (data)
	for(size_t j = 0; j < points; ++j){
		//time from first curve
		file << curves[0].x[j];
		for(const auto& curve : curves){
			double v = (j < curve.y.size()) ? curve.y[j] : 0.0;
			file << "," << v;
		}
		file << "\n";
	}
	file.close();
}

void ProjectPanel::exportRun(const std::shared_ptr<Run>& run, const wxString& path)
{
	if(!run)
		return;

	auto& times = run -> getTimes();
	auto& frames = run -> getFrames();

	if(times.empty() || frames.empty())
		return;

	std::ofstream file(path.ToStdString());
	if(!file.is_open())
		return;

	//header
	file <<"Time";
	for(size_t s = 0; s < frames[0].size(); ++s)
		file << ", " << m_sensors[s] -> getName();
	file << "\n";

	//data
	for(size_t i = 0; i < times.size(); ++i){
		file << times[i];
		for(auto val : frames[i])
			file << "," << val;
		file << "\n";
	}
	file.close();
}

// ========================== UI ==========================

//only needed if sessionController wants to push raw string frames
void ProjectPanel::onNewDataFrame() {
	//ignore if run hasn't started
	if(!m_isRunning || !m_currentRun)
		return;

	//build values vector from current sensor readings
	std::vector<double> values;
	values.reserve(m_sensors.size());

	for(auto& s : m_sensors)
		values.push_back(s -> getReading());


	//compute time relative to the run start

	//get current time in seconds
	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;

	//convert to time since run started
	double t = now - m_runStartTime;

	//store in the current run
	m_currentRun -> addFrame(t, values);


	// ========= SAVE TO DATABASE ========
    	//write frames to the DB
    	if(m_saveProjectToDB && m_currentRunId >= 0){
        	//create the frame row first to get a frame ID
        	int frameId = m_DB->createFrame(m_currentRunId, t);

        	if(frameId >= 0){
            		//save each sensor's value linked to this frame
            		for(size_t i = 0; i < values.size(); i++){
                		int sensorId = m_DB -> getSensorID(m_sensors[i] -> getName());
                		m_DB -> saveFrameValue(frameId, sensorId, values[i]);
            		}
        	}
    	}

	//push this frame to the live graph window
	if(m_liveWindow)
		m_liveWindow -> addFrame(t, values);

	//if graph window already exists update it while the run is going
	if(m_graphWindow)
		graphRun(m_currentRun);
}


void ProjectPanel::onSensors()
{
	//launch the SensorConfigDialog chain
	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_DB, m_sensors);
	dlg.ShowModal();
}


// ================== LOAD PROJECT FROM DB ==============
void ProjectPanel::loadProjectFromDatabase()
{
	projecthasbeenloaded = true;

	std::cout << "DEBUG load: m_projectId=" << m_projectId << "\n";

	// ========= LOAD PROJECT_SENSORS ==========
	//ensure that the pointer exists and a valid id has been assigned
	if(!m_DB || m_projectId < 0)
		return;

	// ========== FULL RESET =========
    	//clear all in-memory state before loading so that calling this function multiple times always produces a clean slate with no duplicate curves or tabs
        if(m_isRunning)
    	    stopRun();

    	if(m_graphWindow)
        	m_graphWindow -> clear();

    	if(m_liveWindow)
        	m_liveWindow -> clearAll();

    	if(m_tableWindow){
        	if(m_bottom_splitter && m_bottom_splitter -> IsSplit())
            		m_bottom_splitter -> Unsplit();
        	m_tableWindow.reset();
    	}

    	m_runs.clear();
    	m_currentRun.reset();
    	m_currentRunId  = -1;
    	m_isRunning     = false;
    	graphVisible      = false;
    	liveVisible       = false;
    	collectNowVisible = false;

	m_DB -> loadProjectSampleRate(m_projectId, m_hz);
	std::cout << "the loaded sample rate is: " << m_hz << "\n";

	//clear existing sensors before loding new ones
	m_sensorManager -> clearSensors();
	m_sensors.clear();

	//we retrieve the sensors that were used in the project from the project_sensors table & save them in a vector <name, pin>
	//each sensor is reconstructed as a sensor object and pushed into m_sensors. SensorManager sees them immediately through its refrence to m_sensors
	std::vector<std::pair<std::string, int>> sensors;

	m_DB -> loadProjectSensors(m_projectId, sensors);

	std::cout << "Sensors retrieved from db: " << sensors.size() << std::endl;

	if(sensors.empty())
		return;

	for(auto& [name, pin] : sensors){
		//push the unique_ptr into m_sensors, SensorManager will see it immediately due to the refrence that it has to m_sensors
		//no second registration step is needed cause there is exactly one unique_ptr per sensor object
		m_sensors.push_back(std::make_unique<Sensor>(name, pin));
	}

	std::cout << "SensorManager now has: " << sensors.size() << " sensors\n";

	//send loaded sensors to microcontroller if connected
	if(m_serial && m_serial->handshakeresult){
   		for(const auto& s : m_sensors)
        		m_serial -> addSensor(s->getName(), s->getPin());
	}


	// ========= UI STATE ==========
        //load user interface state cause the db stores which windows were visible when the project was last saved
        //we load before populating so the user can see the historical data even without running new experiemnt
	bool graph, live, collect;

        m_DB -> loadUIState(m_projectId, graph, live, collect);

        //apply the loaded visibility states to the project panel ones
        graphVisible = graph;
        liveVisible = live;
        collectNowVisible = collect;


	// ======== LOAD ALL PREVIOUS RUNS FROM DB ======
	//(db_run_id, run_number)
	std::vector<std::pair<int,int>> runRows;
	m_DB -> loadProjectRuns(m_projectId, runRows);

	std::cout << "Runs retrieved from db: " << runRows.size() << "\n";

	for(auto& [dbRunId, runNumber] : runRows){
		//create the run object with its original run number
		auto run = std::make_shared<Run>(runNumber);

		//load every frame that belongs to this run
		std::vector<std::pair<double, std::vector<double>>> frames;
		m_DB -> loadRunFrames(dbRunId, m_sensors, frames);

		for(auto& [t, values] : frames)
			run -> addFrame(t, values);

		//store it in the project's run list
		m_runs.push_back(run);

		//draw it on the graph and display it if it has data
		if(!run -> getFrames().empty()){
			//restore the graph curves for this run
        		graphRun(run);

        		//restore the live data notebook tab for this run
        		//startNewRun creates the tab and column headers, then we replay every frame row by row into the grid.
        		m_liveWindow -> addHistoricalRun(run, m_sensors);
    		}

		std::cout << " Run " << runNumber << " loaded with " << run -> getFrames().size() << " frames\n";
	}

	// ======= RESTORE CALIBRATION =======
	/* After sensors are in m_sensors, check if each one has a saved calibration in project_calibrations. If it does,
	reconstruct the interpolator and assign it to the sensor so readings are calibrated immediately when the project loads, if no 
	calibration exists for a sensor, it runs uncalibrated
	*/
	for(size_t i = 0; i < m_sensors.size(); ++i){
    		int sensorId = m_DB -> getSensorID(m_sensors[i] -> getName());
    		int pin = m_sensors[i] -> getPin();

    		std::string type;
    		std::vector<CalibrationPoint> points;

		bool found = m_DB -> loadProjectCalibration(m_projectId, sensorId, pin, type, points);

		if(!found){
			//no project specific calibration, try global template
			found = m_DB -> loadGlobalCalibration(sensorId, type, points);
			if(found)
				std::cout << "Using global calibration for '" << m_sensors[i] -> getName() << "' pin=" << pin << "\n";
		}
		else{
			std::cout << "Using project calibration for '" << m_sensors[i] -> getName() << "' pin=" << pin << "\n";
		}

    		if(found && !points.empty()){
        		//currently only "table" type is implemented, when equation and datasheet are added, we need tobranch on type here
        		auto table = std::make_unique<std::vector<CalibrationPoint>>(points);
        		auto calibrator = std::make_unique<Interpolator>(std::move(table));
       			 m_sensorManager -> setCalibration(i, std::move(calibrator));

        		std::cout << "Calibration restored for '" << m_sensors[i] -> getName() << "' pin=" << pin << "\n";
    		}
	}

	// ========= UI STATE ==========
	//if collect-now was visible, we need m_tableWindow to exist before updateLayout() tries to show it, otherwise the splitter has nothing to display and the layout silently does nothing
    	if(collectNowVisible && !m_runs.empty()){
        	std::vector<std::shared_ptr<DataSession>> sessions;
        	sessions.push_back(std::make_shared<DataSession>("Time"));
        	for(auto& sensor : m_sensors)
            		sessions.push_back(std::make_shared<DataSession>(sensor -> getName()));

		//associate the table with the last loaded run
    	    	m_tableWindow = std::make_unique<DataTableWindow>(m_bottom_splitter.get(), sessions, m_runs.back());

		if(!runRows.empty()){
        		int lastDbRunId = runRows.back().first;

        		std::vector<std::tuple<double,int,double>> points;
       			m_DB -> loadCollectPoints(lastDbRunId, points);

        		std::cout << "Collect points loaded: " << points.size() << "\n";

        		//group by timestamp where one collect event creates one row per sensor
        		std::map<double, std::vector<double>> rowMap;

        		for(auto& [t, sensorId, value] : points)
            			if(rowMap.find(t) == rowMap.end())
                			rowMap[t] = std::vector<double>(m_sensors.size(), 0.0);

        		for(auto& [t, sensorId, value] : points)
            			for(size_t i = 0; i < m_sensors.size(); ++i)
                			if(m_DB -> getSensorID(m_sensors[i] -> getName()) == sensorId){
                    				rowMap[t][i] = value;
                    				break;
                			}

			//restore table rows
        		for(auto& [t, sensorValues] : rowMap){
            			std::vector<double> row;
            			row.push_back(t);
            			row.insert(row.end(), sensorValues.begin(), sensorValues.end());
            			m_tableWindow -> appendRow(row);
        		}

			//restore red circles on graph using the same curve ID
        	       for(auto& [t, sensorValues] : rowMap){
            			for(size_t i = 0; i < sensorValues.size(); ++i){
		  	              std::string curveId = "run" + std::to_string(m_runs.back() -> getRunNumber()) + "_sensor" + std::to_string(i);

                			m_graphWindow -> addDemandPoint(curveId, t, sensorValues[i]);
            			}
        		}

        		std::cout << "Collect points restored: " << rowMap.size() << " rows\n";
    		}
	}

	//update layout
	CallAfter([this](){ updateLayout();});
}


// ======================= THEME ==========================
void ProjectPanel::applyTheme(Theme theme)
{
	m_currentTheme = theme;

	//colors that will be applied to this panel and all child UI elements
	wxColour bg, fg;

	//choose colors based on the selected theme
	if(theme == Theme::Dark){
		//dark mode, dark background with light text
		bg = wxColour(30, 30, 30);
		fg = wxColour(220, 220, 220);
	}
	else if(theme == Theme::Light){
		//light mode: white backgound with black text
		bg = *wxWHITE;
		fg = *wxBLACK;
	}

	//apply the colors to this projectPanel
	SetBackgroundColour(bg);
	SetForegroundColour(fg);

	//propagate the theme change to the data table window (if it exists)
	//this insures that the table matches the main UI theme
	if(m_liveWindow)
		m_liveWindow -> applyTheme(theme);

	if(m_tableWindow)
		m_tableWindow -> applyTheme(theme);

	//we do the same with the graph
       	if (m_graphWindow)
        	m_graphWindow -> setTheme(theme);

    	//force the panel to redraw itself using the new colors
    	Refresh();
}

void ProjectPanel::adjustSampleRate(const int rate) {
	/* \brief	This function handles the event that the button for
* 		adding a new sensor is pressed in the dialog interface.
*
* 		The only thing necessary is to stack allocate an instance
* 		of the AddSensorDialog, passing along the pointer to the
* 		sensorManager owned by the parent Project. Any modification
* 		to the vector of sensors is made by SensorManager, and not
* 		this or any other dialog.
*/

	//add a check to see if we have connected with a device yet
	if (!m_serial->handshakeresult)
	{
		wxMessageBox("Please connect with a microcontroller first!");
		return;
	}


	//convert the desired hz into a millisecond delay value
	m_sampleRate = 1000/rate;

	//set the arduino sampling rate to the required value
	m_serial->adjustPollingRate(m_sampleRate);

	//now update the value being used in the background thread.
	m_controller->setInterval(m_sampleRate);

	//and update the database
	m_DB ->saveProjectSampleRate(m_projectId, m_sampleRate);
}

SensorManager * ProjectPanel::getSensorManager() const {
	return m_sensorManager.get();
}

float ProjectPanel::getSampleRate() const{
	return m_sampleRate;
}
