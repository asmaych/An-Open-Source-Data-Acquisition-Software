#include <wx/msgdlg.h>
#include "ProjectPanel.h"
#include "Events.h"
#include "HandshakeDialog.h"
#include "data/GraphWindow.h"
#include "data/DataTableWindow.h"
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "serial/SerialComm.h"
#include <chrono>
#include <iostream>
#include <thread>

ProjectPanel::ProjectPanel(wxWindow* parent, const wxString& title)
	: wxPanel(parent, wxID_ANY)
{

	//SensorList as a Table
	m_sensorList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 200), wxLC_REPORT | wxLC_SINGLE_SEL);
	
	//Add columns for table
	m_sensorList -> InsertColumn(0, "Sensor Name");
	m_sensorList -> InsertColumn(1, "Type");
	m_sensorList -> InsertColumn(2, "Unit");
	m_sensorList -> InsertColumn(3, "Last Reading");
	
	m_connect_button = new wxButton(this, wxID_ANY, "Connect");

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer -> Add(m_sensorList, 1, wxEXPAND | wxALL, 6);
	mainSizer -> Add(m_connect_button, 0, wxALIGN_CENTER | wxALL, 6);
	SetSizerAndFit(mainSizer);

	//Create SerialComm instance for this project (not connected yet)
	m_serial = std::make_unique<SerialComm>();

	// Bind the Connect button to open the handshake dialog
	m_connect_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
		openConnectDialog();
	});

	// Bind the custom handshake success event to its handler
	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);
}

ProjectPanel::~ProjectPanel()
{
	/* This destructor is explicitly implemented so that the threads 
	 * created during runtime get shut down when the ProjectFrame
	 * lifetime ends
	 */
	stopBackgroundPolling();
}

//open Handshake dialog
void ProjectPanel::openConnectDialog()
{
	//if handshake already done, let the user know
	if (handshakeComplete) {
		wxMessageBox("Already connected to microprocessor.", "Info");
		return;
	}
	//create the dialog and show it
	HandshakeDialog* handshaker = new HandshakeDialog(this, "Select serial port", m_serial.get());
	handshaker->Show();
}

void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
    bool success = evt.GetPayload<bool>(); //evt is a thread event and GetPayload<bool> gets the result of the handshake (T=success)
    if (success)
    {
	handshakeComplete = true;
        wxLogStatus("Handshake successful - ready to poll!");
        // Start polling sensors now that the connection is ready
        startBackgroundPolling();
    }
    else
    {
        wxLogError("Handshake failed!");
    }
}


//Background thread function to poll sensor readings: a loop that periodically requests data from the microprocessor
void ProjectPanel::startBackgroundPolling()
{
        if(!handshakeComplete){
		wxLogWarning("Cannot start polling: not connected");
		return;
	}

	if (running.exchange(true)){
		//polling already active (exchange sets running to true and returns the previous value)
		return;
	}
	
	//we start a new thread that runs in the background [this] lets the thread access the class's members as sensor
        ioThread = std::thread([this]() {
                        while (running.load()) //while running is true (load thread-safe)
                	{
				// we lock the serialComm so only one thread can access the hardware at a time which prevents
				// conflicts if multiple threads try to read/write at once
                               std::lock_guard<std::mutex> lock(serialMutex);
                               // Loop through sensors
            		       for (size_t i = 0; i < sensors.size(); i++) {
         		       		int value = m_serial->getReading(); // poll hardware
             			  	 sensors[i]->setReading(value);      // update the sensor object with the new reading
					
					//send /add the value to all live windows so they display real-time data
					for (auto& window : m_liveWindows) {
					    window->addValue(value);   // direct update
					}
				}
          		// small sleep to avoid overwhelming the arduino/esp32
         		std::this_thread::sleep_for(std::chrono::milliseconds(100));
      			}
        });
}
	

//Stops backgrounf polling thread
void ProjectPanel::stopBackgroundPolling()
{
        running.store(false);
	
	//we check if the background thread actually exists and is running
        if (ioThread.joinable())
        {
		//wait for it to finish its work safely before exiting it
                ioThread.join();
        }
}

//called when serial update event occurs
void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
	int value = evt.GetInt();
	wxLogStatus("Reading updated: %d",value);
	
	long selected = m_sensorList -> GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if( selected != -1){
		//set text in the second column if present, but we are keeping it simple for now
		m_sensorList->SetItem(selected, 1, wxString::Format("%d", value));
	}
}


// Helper to get selected sensor index
int ProjectPanel::getSelectedSensorIndex() const
{
	//we search the list of sensrs from top till we find the first item that is selected
	long item = m_sensorList -> GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	//if no items (-1) return -1 else return the row index of the selected sensor(convert the long returned by getNext to an int)
	if(item == -1){
		return -1;
	}
	return static_cast<int>(item);
}


//	TOOLBAR ACTIONS

//Start live reading
void ProjectPanel::startSelectedSensor()
{
	int index = getSelectedSensorIndex();
	if(index == -1){
		wxMessageBox("Please select a sensor first.", "No sensor", wxICON_INFORMATION);
		return;
	}
	
	 // create a live window for the selected sensor
	// we get the data session for that sensor using m_sessions[index].get (the vector of sessions) 
	// which contains the data session of thesensor
         auto live = std::make_unique<LiveDataWindow>(this, m_sessions[index].get());
         live->Show();
         m_liveWindows.push_back(std::move(live)); //we store it so it won't get deleted automatically
         startBackgroundPolling();
}

//Stop live reading
void ProjectPanel::stopSelectedSensor()
{
        int index = getSelectedSensorIndex();
        if(index == -1){
                wxMessageBox("Please select a sensor first.");
                return;
        }
       stopBackgroundPolling();
	wxLogStatus("Stopped sensor polling."); 
        //ASMAAAA COMEBACKKKK
}

//Collect data
void ProjectPanel::collectSelectedSensor()
{
        int index = getSelectedSensorIndex();
        if(index == -1){
               wxMessageBox("Please select a sensor first.", "No sensor", wxICON_INFORMATION);
                return;
        }
        // we first check if there is a session at the selected index(m_sessions.size()>index
	// and if that session actually exists (not null) the implmentation is vice versa
	if (m_sessions.size() <= static_cast<size_t>(index) || !m_sessions[index]) {
		wxMessageBox("No data collected yet for this sensor!", "No data", wxICON_INFORMATION);
		return;
	}
        //open a data table window and pass the pointer to the sata session so the table knows which values to display
	DataTableWindow* table = new DataTableWindow(this,m_sessions[index].get());
	table -> Show();
}



//Graph data
void ProjectPanel::graphSelectedSensor()
{
        int index = getSelectedSensorIndex();
        if(index == -1){
                wxMessageBox("Please select a sensor first.", "No sensor", wxICON_INFORMATION);
                return;
        }

	if (m_sessions.size() <= static_cast<size_t>(index) || !m_sessions[index]) {
                wxMessageBox("No data collected yet for this sensor!", "No data", wxICON_INFORMATION);
                return;
        }
        
        //Open a graph window
	GraphWindow* graph = new GraphWindow(this, m_sessions[index].get());
	graph -> Show();
}

//Open panel with add/remove sensor buttons
void ProjectPanel::openSensorPanel()
{
	// Simple dialog: if there are no sensors, ask to add one
        if (sensors.empty()) {
     	        // create a sensor with a simple name
        	auto s = std::make_unique<Sensor>("Sensor1", 1);
        	sensors.push_back(std::move(s));

		auto session = std::make_unique<DataSession>("Sensor1");
		m_sessions.push_back(std::move(session));

       		long index = m_sensorList->InsertItem(m_sensorList->GetItemCount(), "Sensor1");
        	wxLogStatus("Sensor1 added.");
       		return;
        }

    	// Otherwise prompt user to add numbered sensor
   	long nextId = static_cast<long>(sensors.size()) + 1;
  	wxString name = wxString::Format("Sensor%ld", nextId);
   	auto s = std::make_unique<Sensor>(std::string(name.mb_str()), static_cast<int>(nextId));
    	m_sensorList->InsertItem(m_sensorList->GetItemCount(), name);
    	sensors.push_back(std::move(s));
    	auto session = std::make_unique<DataSession>(std::string(name.mb_str()));
	m_sessions.push_back(std::move(session));
	wxLogStatus("Added %s", name);
}

//Add/Remove sensor
void ProjectPanel::onAddSensor(wxCommandEvent& evt) 
{
	openSensorPanel();
}


void ProjectPanel::onRemoveSensor(wxCommandEvent& evt) 
{
	// we remove both the sensor and its session (also in the gui)
	int index = getSelectedSensorIndex();
	if(index==-1){
		wxMessageBox("Select a sensor to remove.", "No sensor", wxICON_INFORMATION);
		return;
	}
	
	sensors.erase(sensors.begin() + index);
	if(m_sessions.size() > static_cast<size_t>(index)){
		m_sessions.erase(m_sessions.begin() + index);
	}
	m_sensorList -> DeleteItem(index);
	wxLogStatus("Sensor removed.");
}


