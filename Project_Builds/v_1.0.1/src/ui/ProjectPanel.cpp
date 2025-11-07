#include <wx/wx.h>

#include "HandshakeDialog.h"
#include "SerialComm.h"

#include <thread>
#include <atomic>

#include "LEDPanel.h"

#include "Events.h"

wxDEFINE_EVENT(wxEVT_SERIAL_UPDATE, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_HANDSHAKE, wxThreadEvent);

#include "ProjectPanel.h"

#include <mutex>


ProjectPanel::ProjectPanel(wxWindow* parent, const wxString& title)
	: wxPanel(parent, wxID_ANY)
{

	//Each Project should have an instance of the SerialComm Class:
	serialComm = std::make_unique<SerialComm>();

	//make a sizer for the controls
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//make the status LED display panel
	ledIndicator = new LEDPanel(this);

	
	//make a button that opens the dialogue for handshake
	wxButton* connect_button =  new wxButton
		(
		 this,
		 wxID_ANY,
		 "Connect to sensor controller"
		);

	mainSizer->Add(connect_button, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
	
	//mainSizer->Add(ledIndicator, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
	
	//Sensor Management buttons: add/remove
	wxBoxSizer* sensorButtons = new wxBoxSizer(wxHORIZONTAL);
	wxButton* addSensor = new wxButton(this, wxID_ANY, "Add Sensor");
	wxButton* removeSensor = new wxButton(this, wxID_ANY, "Remove Sensor");
	sensorButtons->Add(addSensor, 0, wxALL, 5);
	sensorButtons->Add(removeSensor, 0, wxALL, 5);
	mainSizer->Add(sensorButtons, 0, wxALIGN_CENTER_HORIZONTAL);

	//SensorList as a Table
	sensorList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(500, 200), wxLC_REPORT | wxLC_SINGLE_SEL);
	
	//Add columns for table
	sensorList -> InsertColumn(0, "Name");
	sensorList -> InsertColumn(1, "Type");
	sensorList -> InsertColumn(2, "Unit");
	sensorList -> InsertColumn(3, "Last Reading");

	//the two main buttons for adding and removing a sensor
	mainSizer->Add(sensorList, 0, wxALL | wxEXPAND, 10);
	mainSizer->Add(ledIndicator, 0, wxALL | wxEXPAND, 10);

	//bind buttons
	connect_button->Bind(wxEVT_BUTTON, &ProjectPanel::onHandshake,this);
	addSensor->Bind(wxEVT_BUTTON, &ProjectPanel::onAddSensor,this);
	removeSensor->Bind(wxEVT_BUTTON, &ProjectPanel::onRemoveSensor,this);

	//bind the custom event to its handler
	Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectPanel::onHandshakeSuccess, this);

	SetSizerAndFit(mainSizer);

}

ProjectPanel::~ProjectPanel()
{
	/* This destructor is explicitly implemented so that the threads 
	 * created during runtime get shut down when the ProjectFrame
	 * lifetime ends
	 */
	stopBackgroundPolling();
}

// AddSensor button clicked
void ProjectPanel::onAddSensor(wxCommandEvent& evt)
{
	//create a new sensor object, each roject has its own sensors
	auto sensor = std::make_unique<Sensor>("Sensor"+ std::to_string(sensors.size() + 1), sensors.size()+1);
	
	//Append to vector
	sensors.push_back(std::move(sensor));

	//Update table visually
	long index = sensorList->InsertItem(sensorList->GetItemCount(), sensors.back()->getName());
	//sensorList->SetItem(index, 1, sensors.back()->getType());
        //sensorList->SetItem(index, 2, sensors.back()->getUnit());
        sensorList->SetItem(index, 3, "N/A");
}



//RemoveSensor button clicked
void ProjectPanel::onRemoveSensor(wxCommandEvent& evt)
{
	long selected = sensorList -> GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	/* GetNextItem is a method of wxListCntrl (that displays the table), GetNextItem(long item, int geometry, int state)
	   item is the starting index, if u pass -1 that means the search should start from the top of the list (first item)
	   geometry tells it where to search next, usually we use wxLIST_NEXT_ALL which means search all items
	   state is a filter telling it what kind of item we are looking for - wx_STATE_SELECTED means return the index of the first
           selected items
	=> here it is saying look through all items in sensorList, find the first one that's currently selected, if found
	   (not equal 1) remove that sensor object from the internal sensor vec,emove its row from visual list
	*/
	if(selected != -1){
		sensors.erase(sensors.begin() + selected);
	/* erase is a method that removes the sensor pointed to, we are passing sensors.begin()+selected because sensors.begin()
	   returns an iterator pointing to the first element of the list/vec while §selected moves that pointer to the selected 
 	   sensor to erase it
	*/
		sensorList->DeleteItem(selected);
	//we delete it visually
	}
}

//open Handshake dialog
void ProjectPanel::onHandshake(wxCommandEvent& evt)
{
	wxLogStatus("Initiating handshake dialogue:");

	//TODO
	//now instantiate the thing that does the handshaking
	//
	//Pass a raw pointer to the SerialComm object here, to avoid transferring ownership
	HandshakeDialog* handshaker = new HandshakeDialog(this, "Handshaker", serialComm.get());
	handshaker->Show();
}

//called when handshake successful 
void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
        bool success = evt.GetPayload<bool>();
        if (success)
        {
                //prompt the arduino to move to the next phase
                //std::lock_guard<std::mutex> lock(serialMutex);
                //serialComm->writeData("Begin\n");

                //std::cout << "\n\n" << "test" << "\n\n";
                startBackgroundPolling();
        }
}

//Background thread function to poll sensor readings: a loop that periodically requests data from the microprocessor
void ProjectPanel::startBackgroundPolling()
{
        //set the class variable boolean to true
        running = true;

        ioThread = std::thread([this]()
                {
                        while (running)
                {
                                std::cout << "thread iteration: we call SerialComm::writeData()\n";

                                std::lock_guard<std::mutex> lock(serialMutex);
                                //loop through all sensors and get readings
				for(size_t i = 0; i < sensors.size(); i++){
					 int value = serialComm->getReading(); //poll hardware
					 sensors[i]->setReading(value);  //update model
                                		
					 //update GUI - trip the event and run the update:
                               		  wxThreadEvent evt(wxEVT_SERIAL_UPDATE);
                                	  evt.SetInt(value);
                                	  wxQueueEvent(this, evt.Clone());
				}
                                 //add a little pause
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                        }
                });
}


//Stops backgrounf polling thread
void ProjectPanel::stopBackgroundPolling()
{
        running = false;

        if (ioThread.joinable())
        {
                ioThread.join();
        }
}

//called when serial update event occurs
void ProjectPanel::onSerialUpdate(wxThreadEvent& evt)
{
	int value = evt.GetInt();

	if (value == 1)
	{
		ledIndicator->setState(true);
	}
	else if (value == 0)
	{
		ledIndicator->setState(false);
	}
	else
		wxLogStatus("Reading updated: %d",value);
	
	//TODO: UPdate table values if needed
}


