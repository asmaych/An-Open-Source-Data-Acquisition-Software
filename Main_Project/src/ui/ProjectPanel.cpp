#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include "ProjectPanel.h"
#include "Events.h"
#include "HandshakeDialog.h"
#include "data/GraphWindow.h"
#include "data/DataTableWindow.h"
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "sensor/SensorManager.h"
#include "serial/SerialComm.h"
#include "SensorSelectionDialog.h"
#include <chrono>
#include <iostream>
#include <thread>

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

	//create data collector
	m_collector = std::make_unique<DataCollector>();
	m_collector -> setMode(CollectionMode::Continuous);

	//Bind events
	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);
	//Bind(wxEVT_OPEN_SENSOR_GRAPH, &ProjectPanel::onGraph, this);
}


ProjectPanel::~ProjectPanel()
{
	/* This destructor is explicitly implemented so that the threads 
	 * created during runtime get shut down when the ProjectFrame
	 * lifetime ends
	 */
	stopPollingIfNeeded();

	for(auto* graph: m_graphWindows){
		delete graph;
	}
	
	m_graphWindows.clear();
	
	//delete the single collect window
	if(m_tableWindow){
		m_tableWindow -> Destroy();
		m_tableWindow = nullptr;
	}
}


int ProjectPanel::getSelectedSensorIndex() const
{
	long index = m_sensorList -> GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(index == -1){
		return -1;
	}
	return static_cast<int>(index);
}


void ProjectPanel::refreshSensorList()
{
	m_sensorList -> DeleteAllItems();
	for(size_t i = 0; i < m_sensors.size(); ++i){
		auto* s = m_sensors[i].get();
		long index = m_sensorList -> InsertItem(i, s -> getName());
		m_sensorList -> SetItem(index, 1, std::to_string (s -> getPin()));
		ensureSessionForSensor(i);
	}
}

//open Handshake dialog
void ProjectPanel::openConnectDialog()
{
	//if handshake already done, let the user know
	if (handshakeComplete) {
		wxMessageBox("Already connected to microprocessor.", "Info");
		return;
	}
	//stack allocate the dialog, and show it:
	HandshakeDialog handshaker(this, "Select serial port", m_serial.get());
	handshaker.ShowModal();
}


void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
	//evt is a thread event, and GetPayload<bool> gets the result of the handshake (T = success)
 	bool success = evt.GetPayload<bool>();
    	if (success)
    	{
		handshakeComplete = true;
        	wxLogStatus("Handshake successful - ready to poll!");
        
		// Start polling sensors now that the connection is ready
        	m_controller = std::make_unique<SessionController>(m_serial.get(), &m_sessions, m_collector.get(), this, &m_sensors);
    	}
    	else
    	{
        	wxLogError("Handshake failed!");
    	}
}

void ProjectPanel::toggleStartStop()
{
	if(!handshakeComplete){
		wxMessageBox("Please connect first.", "Warning");
		return;
	}

	if(!isRunning){
		//TRYING SMTH
		if(m_serial){
			m_serial -> writeString("start");
		}

		startPollingIfNeeded();
		wxLogStatus("Started Polling");

		//update toolbar button from start to stop
		if(m_mainFrame){
			m_mainFrame -> setStartToggleToStop();
		}
		isRunning = true;
	}

	else{

		stopPollingIfNeeded();
		wxLogStatus("Stopped polling");

		//here as well
                if(m_serial){
                        m_serial -> writeString("stop");
                }


		//update toolbar
		if(m_mainFrame){
			m_mainFrame -> setStartToggleToStart();
		}
		isRunning = false;
	}

}


void ProjectPanel::startPollingIfNeeded()
{
	/* \brief 	This function is used to open up a background thread
	 * 		whose only purpose is to read the incoming dataframes
	 * 		from the connected microcontroller, parse each frame,
	 * 		and assign sensor readings to all objects in the 
	 * 		class member m_sensors.
	 */

	if(!handshakeComplete)
		return;
	if(isRunning)
		return;

	//populate activeSensors with the current sensor pointers
	m_activeSensors.clear();
	for(auto& sensor: m_sensors) {
		m_activeSensors.push_back(sensor.get());
	}

	//create LiveDataWindow if it doesn't exist
	if(!m_liveWindow){
		m_liveWindow = std::make_unique<LiveDataWindow>(this, m_activeSensors);
		m_liveWindow -> Show();
	}

	if (m_controller){
		m_controller -> start();
	}

	isRunning = true;
}


void ProjectPanel::stopPollingIfNeeded()
{
	/* \brief the purpose of this method is to safely shut down the 
	 * background polling of sensor data from the microcontroller. 
	 *
	 *
	 */

	if(m_controller){
		m_controller -> stop();
	}

	isRunning = false;
}


void ProjectPanel::ensureSessionForSensor(size_t index)
{
	if(index >= m_sessions.size()){
		while(m_sessions.size() <= index) {
			//use the actual sensor name
			std::string sensorName = m_sensors[m_sessions.size()] -> getName();
			m_sessions.push_back(std::make_unique<DataSession>(sensorName));
		}
	}
}

void ProjectPanel::collectContinuous()
{
	//if no sensors selected yet or table not created
	if(m_selectedContinuousIndexes.empty())
	{
		//check if there are any sensors in the project
		if(m_sensors.empty()){
			wxMessageBox("No sensors available in project.");
			return;
		}

		//prepare a list of sensor names for the selection dialog
		std::vector<std::string> names;
		for(auto& sensor : m_sensors){
			names.push_back(sensor -> getName());
		}

		//create and show the sensor selection dialog
		SensorSelectionDialog dlg(this, names);

		//exit if user cancels
		if(dlg.ShowModal() != wxID_OK){
			return;
		}

		//get indexes of sensors selected by the user
		m_selectedContinuousIndexes = dlg.getSelectedIndexes();

		//warn if none selected
		if(m_selectedContinuousIndexes.empty()){
			wxMessageBox("No sensors selected");
			return;
		}
	}

	if(!m_liveWindow){
		wxMessageBox("Live window not running. Start polling first.");
		return;
	}

	//read all buffered values from live window
	std::vector<double> bufferedValues = m_liveWindow -> getBufferedValues();

	//if no new values were received/read
	if(bufferedValues.empty()){
		wxLogStatus("No new data to collect");
		return;
	}

	//create rows per frame where each row corresponds to one frame of readings from all selected sensors
	size_t sensorCount = m_selectedContinuousIndexes.size();
	std::vector<std::vector<double>> rows;

	for(size_t i = 0; i < bufferedValues.size(); i += sensorCount){
		std::vector<double> row;

		for(size_t j = 0; j < sensorCount && (i + j) < bufferedValues.size(); ++j){
			int sensorIndex = m_selectedContinuousIndexes[j];

			//add the sensor value to the current row
			row.push_back(bufferedValues[i + j]);

			//ensure a session exists for this sensor
			ensureSessionForSensor(sensorIndex);

			//add the value to the correspondng session for data storage
			m_sessions[sensorIndex] -> addValue(bufferedValues[i + j]);
		}
	//store completed frame now
	rows.push_back(row);
	}

	//prepare dataSessions for the table window
	std::vector<std::shared_ptr<DataSession>> selectedSessions;
	for(int index : m_selectedContinuousIndexes){
		//create shared_ptr wrapper without deleting original datasession
		selectedSessions.push_back(std::shared_ptr<DataSession>(m_sessions[index].get(), [](DataSession*){}));
	}

	//show or update the continuous data table
	if(!m_continuousTableWindow){
		//if the table doesn't exist yet, create it & display it
		m_continuousTableWindow = new DataTableWindow(this, selectedSessions);
		m_continuousTableWindow -> Show();
	}

	//append collected rows to the table
	for(auto& row: rows){
		m_continuousTableWindow -> appendRow(row);
	}

	//clear the buffer in the live window so that next collection only grabs new data
	m_liveWindow -> clearBuffer();

	//log status for the user
	wxLogStatus("Collected continuous data into sessions.");
}


void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
	auto readings = evt.GetPayload<std::vector<int>>();

    	for(size_t i=0; i<readings.size() && i<m_sessions.size(); ++i){
		ensureSessionForSensor(i);
		
		double value = static_cast <double>(readings[i]);

		//store once in dataSession
		m_sessions[i] -> addValue(value);

		//show only first sensor live (simple & safe)
		if (m_liveWindow && i == 0)
			m_liveWindow -> addValue(value); 
	}
}


void ProjectPanel::collectCurrentValues()
{
	//check if no table created
	if(m_selectedCurrentIndexes.empty() || !m_tableWindow){
		
		if(m_sensors.empty()){
			wxMessageBox("No sensors to collect.");
			return;
		}

		//let the user pick sensors to collect
		std::vector<std::string> names;
		for(auto& s : m_sensors){
			names.push_back( s -> getName());
		}

		SensorSelectionDialog dlg(this, names);
		if(dlg.ShowModal() != wxID_OK){
			return;
		}

		m_selectedCurrentIndexes = dlg.getSelectedIndexes();

		if(m_selectedCurrentIndexes.empty()){
			wxMessageBox("No sensors selected");
			return;
		}
	}

	//read latest value for each selected sensor and append to its session
	std::vector<double> rowValues;
	rowValues.reserve(m_selectedCurrentIndexes.size());

	for(int index : m_selectedCurrentIndexes){
		ensureSessionForSensor(index);
		//read latest values - if sensor is valid, get reading; otherwise, default to 0
		double latest = (m_sensors[index] ? m_sensors[index] -> getReading() : 0);
	
		//add to corresponding session
		m_sessions[index] -> addValue(latest); 
	
		rowValues.push_back(latest);
	}

	//prepare DataSessions for the grid/table
	std::vector<std::shared_ptr<DataSession>> selectedSessions;
	for(int index : m_selectedCurrentIndexes){
		selectedSessions.push_back(std::shared_ptr<DataSession>(m_sessions[index].get(), [](DataSession*){}));
	}

	//update the DataTableWindow if it exists
	if(!m_tableWindow){
		m_tableWindow = new DataTableWindow(this, selectedSessions);
		m_tableWindow -> Show();
	}/*else{
		m_tableWindow -> setSelectedSessions(selectedSessions);
	}*/

	//append row
	m_tableWindow -> appendRow(rowValues);

	//log status for user feedback
	wxLogStatus("Collected last values into sessions.");
}


void ProjectPanel::graphSelectedSensor(wxCommandEvent& evt)
{
	int sensorIndex = evt.GetInt();

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
}


void ProjectPanel::exportSessions()
{
	//get the index of the currently selected sensor in the user interface
	int index = getSelectedSensorIndex();

	/* check if the index is valid:
		1. a sensor is actually selected
		2. the session exists in the vector
		3. the dataSession pointer is valid
	*/
	if(index < 0 || m_sessions.size() <= static_cast<size_t>(index) || !m_sessions[index])
		return;

	/*we use a file dialog to ask the user where to save the CSV file
		this : parent window
		"save csv" : our dialog title
	        "" : default directory (empty means current working directory)
		"CSV files (*.csv) | * .csv": filter to show only csv files (no other files)
		wxFD_SAVE | ...: flags to save nd warn if overwriting an existing file
	*/
	wxFileDialog dlg(this, "Save CSV", "", "", "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	//show the dialog and check is the user presses OK -> if not cancel
	if(dlg.ShowModal() != wxID_OK)
		return;

	/*Export the selected Datasession to a csv file
		1. dlg.GetPath(): path chosen by the user
		2. m_sessions[index].get(): raw pointer to the dataSession to export
	we return true if seccessful, else false
	*/
	if(!ExportManager::exportSessionToCSV(std::string(dlg.GetPath().mb_str()), m_sessions[index].get()))
		wxMessageBox("Export failed", "Error", wxICON_ERROR);
}



void ProjectPanel::resetSessionData()
{
	if (m_controller){
		m_controller->reset(m_sessions);
    	}

	if(m_liveWindow){
		m_liveWindow -> clearDisplay();
	}

	wxLogStatus("Live window and sessions reset.");
}


void ProjectPanel::onSensors()
{
	//launch the SensorConfigDialog chain
	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_sensors);
	dlg.ShowModal();
}

void ProjectPanel::openSensorPanel()
{
    // TODO: implement sensor panel opening logic
}

void ProjectPanel::onNewDataFrame(const std::string& frame)
{
	//update liveDataWindow with latest readings
	if(m_liveWindow)
	{
		for(Sensor* sensor : m_activeSensors)
		{
			//push each sensor's current reading into liveDataWindow
			m_liveWindow -> addValue(sensor -> getReading());
		}
	}
}
