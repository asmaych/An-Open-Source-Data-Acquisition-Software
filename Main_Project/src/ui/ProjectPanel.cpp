#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/datetime.h>
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
#include "serial/SerialComm.h"
#include "SensorSelectionDialog.h"
#include "data/Run.h"
#include "controllers/SessionController.h"
#include "MainFrame.h"

ProjectPanel::ProjectPanel(wxWindow* parent, const wxString& title)
	: wxPanel(parent, wxID_ANY)
{
	//basic UI: sensor List + connect button
	m_sensorList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	m_sensorList -> InsertColumn(0, "Sensor Name");
	m_sensorList -> InsertColumn(1, "Pin");

	m_connect_button = new wxButton(this, wxID_ANY, "Connect");
	m_connect_button -> Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
		openConnectDialog();
	});

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer -> Add(m_sensorList, 0, wxEXPAND | wxALL, 5);
	mainSizer -> Add(m_connect_button, 0, wxALIGN_CENTER | wxALL, 5);
	SetSizerAndFit(mainSizer);

	//Create SerialComm instance for this project (not connected yet)
	m_serial = std::make_unique<SerialComm>();

	//create a SensorManager class and point sensorManager to it
	//Note that the sensorManager is given the address of the class
	//member m_sensors vector in order to modify it
	m_sensorManager = std::make_unique<SensorManager>(m_sensors, m_serial.get());

	//set a callback so ui updates automatically when sensors change
	m_sensorManager -> setOnChangeCallback([this](){
		refreshSensorList();
	});

	m_liveWindow = std::make_unique<LiveDataWindow>(this);

	//Bind events
	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);
}


ProjectPanel::~ProjectPanel()
{
	//make sure no run is left active when panel is destroyed
	stopRun();
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

	//Create the session controller
	m_controller = std::make_unique<SessionController>(m_serial.get(), &m_runs, this, &m_sensors);

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

		//update toolbar button
		if(m_mainFrame){
			m_mainFrame -> setStartToggleToStop();
		}

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

		//update toolbar button
		if(m_mainFrame){
			m_mainFrame -> setStartToggleToStart();
		}

		wxLogStatus("Run stopped");
	}
}


//starts a new continuous acquisition run taht stores timestamps and frmaes (vector of sensor values)
void ProjectPanel::startRun()
{
	//create a new run with an incrementing run number
	m_currentRun = std::make_shared<Run>(m_runs.size() + 1);
	m_runs.push_back(m_currentRun);

	//store absolute OS time so we can convert to "run time"
	m_runStartTime = wxGetUTCTimeMillis().ToDouble() / 1000.0;

	//tell the live window to create a new tab for this run
	m_liveWindow -> startNewRun(m_currentRun);
	m_liveWindow -> Show();

	m_isRunning = true;
}


//stops the current run and the data remains stored in m_run
void ProjectPanel::stopRun()
{
	if(!m_isRunning)
		return;

	//stop live display from appending new data
	m_liveWindow -> stopRun();

	//release the active run
	m_currentRun.reset();
	m_isRunning = false;
}

// ========================= DATA FLOW ==========================

//called whenever the microcontroller sends a new frame of data
void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
	//ignore data if no run is active
	if(!m_isRunning || !m_currentRun)
		return;

	auto values = evt.GetPayload<std::vector<double>>(); //where each element is a sensor value

	//current OS time in seconds
	double now = wxGetUTCTimeMillis().ToDouble() / 1000.0;

	//convert OS time into "time since this run started"
	double t = now - m_runStartTime;

	//store permanently in the run object
	m_currentRun -> addFrame(t, values);

	//display it
	m_liveWindow -> addFrame(t, values);
}


// ======================= COLLECT ON DEMAND =====================

//takes the most recent frame from the active run and plug it into a table 
void ProjectPanel::collectCurrentValues()
{
	//check if there is an active run
	if(!m_currentRun){
		wxMessageBox("No active run to collect from!", "Warning");
		return;
	}

	//check if a collect on demand table exists
	if(m_tableWindow){
		//compare if this table is already collecting from this run
		if(m_tableWindow -> getAssociatedRun() != m_currentRun){
			int answer = wxMessageBox("A collect-on-demand table is already open.\n"
						  "Switching to the different run will clear the old table.\n"
						  "Proceed?",
						  "Confirm",
						  wxYES_NO | wxICON_QUESTION);

			if(answer == wxNO)
				return; //user canceled

			//destroy old table
			m_tableWindow -> Destroy();
			m_tableWindow = nullptr;
		}
	//else table already collecting from this run -> append new row
	}

	//get latest frame
	auto& times = m_currentRun -> getTimes();
	auto& frames = m_currentRun -> getFrames();

	if(times.empty() || frames.empty()){
		wxMessageBox("No data available in the current run.", "Info");
		return;
	}

	size_t lastIndex = times.size() - 1;
	double timestamp = times[lastIndex];
	const std::vector<double>& sensorValues = frames[lastIndex];

	//create table window if it doesn't exist
	if(!m_tableWindow){
		std::vector<std::shared_ptr<DataSession>> sessions;
		sessions.push_back(std::make_shared<DataSession>("Time")); //first column
		for(auto& s : m_sensors)
			sessions.push_back(std::make_shared<DataSession>(s -> getName()));

		m_tableWindow = std::make_unique<DataTableWindow>(this, sessions, m_currentRun);
		m_tableWindow -> applyTheme(m_currentTheme); //inherit theme
		m_tableWindow -> Show();
	}

	//append row: timestamp + sensor values
	std::vector<double> row;
	row.push_back(timestamp);
	row.insert(row.end(), sensorValues.begin(), sensorValues.end());

	m_tableWindow -> appendRow(row);
}


// ============================ RESET ============================

//clears all runs and all live data, everything starts from scratch
void ProjectPanel::resetSessionData()
{
	m_runs.clear();
	m_currentRun.reset();

	//clear all tabs in the display
	if(m_liveWindow)
		m_liveWindow -> clearAll();

	//clear the snapshot table
	if(m_tableWindow)
	{
		m_tableWindow -> Destroy();
		m_tableWindow = nullptr;
	}

	wxLogStatus("all data cleared");
}


// ============================= UI =================================

//resreshes the sensor list in the ui, called whenever sensors are added or removed
void ProjectPanel::refreshSensorList()
{
	m_sensorList -> DeleteAllItems();

	for(size_t i = 0; i < m_sensors.size(); ++i){
		auto* s = m_sensors[i].get();
		long index = m_sensorList -> InsertItem(i, s -> getName());
		m_sensorList -> SetItem(index, 1, std::to_string(s -> getPin()));
	}
}


// ============================= GRAPH ===========================
void ProjectPanel::graphSelectedSensor(wxCommandEvent& evt)
{
	// Decide source - is it collect on demand or active run
    	if (m_tableWindow) {
        	//graph collect on demand table
        	graphTable(m_tableWindow.get());
	}

	//otherwise, graph the current run
	else if (m_currentRun){
		graphRun(m_currentRun);
	}

	//otherwise, no data
	else {
		wxMessageBox("No data available to graph!", "Info", wxOK | wxICON_INFORMATION);
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

// FOR NOW I AM GRAPHING ONLY THE RUNS (TO BE DISCUSSED WITH THE CLIENT)
/*
	// =============== GET SESSIONS AND TIMESTAMPS ==============
	//each column in the table is a sensor in collect on demand
	auto sessions = table -> getSelectedSessions();
	auto timestamps = table -> getTimestamps();

	//loop over sensors
	for(size_t col = 0; col < sessions.size(); ++col){
		std::vector<double> values;

		//collect the sensor values from each row
		for(size_t row = 0; row < timestamps.size(); ++row){
			values.push_back(sessions[col] -> getValueAt(row));
		}

		//create a new graph window for each sensor
		auto graphWin = new GraphWindow(this, timestamps, values, sessions[col] -> getSensorName());
		graphWin -> Show();

		//keep track of graph windows
		m_graphWindows.push_back(graphWin);
	}
*/
}



// ========================== GRAPH RUN ============================
void ProjectPanel::graphRun(std::shared_ptr<Run> run)
{
	if(!run || run -> getFrames().empty())
		return;

	//create window if not open
	if(!m_graphWindow)
	{
		m_graphWindow = std::make_unique<GraphWindow>(this);
		m_graphWindow -> setTheme(m_currentTheme); //inherit theme
		m_graphWindow -> Show();
	}

	auto& times = run -> getTimes();  //vector<double> of times
	auto& frames = run -> getFrames(); //vector<vector<double>>: rows = time, columns = sensors

	size_t sensorCount = frames[0].size();

	// ================== LOOP OVER SENSORS ===============
	//each sensor gets its own curve
	for(size_t sensor = 0; sensor < sensorCount; ++sensor){
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
		m_graphWindow -> addCurve(times, y, name, id);
	}
}


// ======================= GETTERS =========================
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


// ======================== EXPORT ==========================
void ProjectPanel::exportSessions(wxCommandEvent& evt)
{
	ProjectPanel* panel = this;
	if(!panel){
		wxMessageBox("No active project to export!", "Error", wxOK | wxICON_ERROR);
		return;
	}

	wxString path = askSaveFile(this);
	if(path.IsEmpty())
		return;

	//if a graph exists, export it with its table
	if(panel -> getGraphWindow()) {
		exportGraph(panel -> getGraphWindow(), path);
		m_graphWindow -> exportImage(path + "_graph.png");
	}

	//else if only table exists, export it
	else if(panel -> getTableWindow()) {
		exportTable(panel -> getTableWindow(), path);
	}

	//else export the current run
	else if(panel -> getCurrentRun()) {
		exportRun(panel -> getCurrentRun(), path);
	}

	else {
		wxMessageBox("No data available to export!", "Error", wxOK | wxICON_ERROR);
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
	if(!table | m_sensors.empty())
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
	for(auto& sensor : m_sensors)
		file << "," << sensor -> getName();
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
	file <<"Time,";
	for(size_t s = 0; s < frames[0].size(); ++s)
		file << m_sensors[s] -> getName();
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

//helper to parse a csv string from serial frame meaning to go from "23.4,50.1,1013" to vector <double> {23.4, 50.1, 1013}
std::vector<double> ProjectPanel::parseSerialFrame(const std::string& frame)
{
	std::vector<double> values;
	std::stringstream ss(frame);
	std::string item;

	//split by comma
	while (std::getline(ss, item, ','))
	{
		try{
			//convert to double
			values.push_back(std::stod(item));
		}

		catch (const std::exception& e){
			//if parsing fails, store 0.0
			values.push_back(0.0);
		}
	}

	return values;
}

//only needed if sessionController wants to push raw string frames
void ProjectPanel::onNewDataFrame(const std::string& frame)
{
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
	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_sensors);
	dlg.ShowModal();
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
	else{
		//light mode: white backgound with black text
		bg = *wxWHITE;
		fg = *wxBLACK;
	}

	//apply the colors to this projectPanel
	SetBackgroundColour(bg);
	SetForegroundColour(fg);

	//propogate the theme change to the data table window (if it exists)
	//this insures that the table matches the main UI theme
	if(m_tableWindow)
		m_tableWindow -> applyTheme(theme);

	//we do the same with the graph
       	if (m_graphWindow)
        	m_graphWindow -> setTheme(theme);

    	//force the panel to redraw itself using the new colors
    	Refresh();
}
