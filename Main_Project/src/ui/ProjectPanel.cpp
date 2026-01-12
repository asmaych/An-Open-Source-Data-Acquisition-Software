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
	wxMessageBox("graphingg");
/*	int sensorIndex = evt.GetInt();

	if(sensorIndex < 0 || sensorIndex >= (int)m_sensors.size()){
		wxMessageBox("Select a sensor first.");
		return;
	}

	auto values = m_sessions[sensorIndex] -> getValues();
	auto timestamps = m_sessions[sensorIndex] -> getTimestamps();

	if(values.empty()){
		wxMessageBox("No data collected for this sensor yet.", "Info");
		return;
	}

	GraphWindow* graph = new GraphWindow(this, timestamps, values, m_sensors[sensorIndex] -> getName());
	graph -> Show();

	//m_graphWindows.push_back(graph);
*/
}


// ======================== EXPORT ==========================
void ProjectPanel::exportSessions()
{
	wxMessageBox("exportingg");
	//get the index of the currently selected sensor in the user interface
/*	int index = getSelectedSensorIndex();
*/
	/* check if the index is valid:
		1. a sensor is actually selected
		2. the session exists in the vector
		3. the dataSession pointer is valid
	*/
/*	if(index < 0 || m_sessions.size() <= static_cast<size_t>(index) || !m_sessions[index])
		return;
*/
	/*we use a file dialog to ask the user where to save the CSV file
		this : parent window
		"save csv" : our dialog title
	        "" : default directory (empty means current working directory)
		"CSV files (*.csv) | * .csv": filter to show only csv files (no other files)
		wxFD_SAVE | ...: flags to save nd warn if overwriting an existing file
	*/
/*	wxFileDialog dlg(this, "Save CSV", "", "", "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	//show the dialog and check is the user presses OK -> if not cancel
	if(dlg.ShowModal() != wxID_OK)
		return;
*/
	/*Export the selected Datasession to a csv file
		1. dlg.GetPath(): path chosen by the user
		2. m_sessions[index].get(): raw pointer to the dataSession to export
	we return true if seccessful, else false
	*/
/*	if(!ExportManager::exportSessionToCSV(std::string(dlg.GetPath().mb_str()), m_sessions[index].get()))
		wxMessageBox("Export failed", "Error", wxICON_ERROR);
*/
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

}


void ProjectPanel::onSensors()
{
	//launch the SensorConfigDialog chain
	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_sensors);
	dlg.ShowModal();
}
