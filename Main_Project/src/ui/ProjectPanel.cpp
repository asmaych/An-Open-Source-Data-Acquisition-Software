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
	//---------------------------------------------------------------------
	//INSTANTIATE CLASS MEMBERS
	//---------------------------------------------------------------------
	
	//Create SerialComm instance for this project (not connected yet)
	m_serial = std::make_unique<SerialComm>();

	//create a SensorManager class and point sensorManager to it
	//Note that the sensorManager is given the address of the class
	//member m_sensors vector in order to modify it
	m_sensorManager = std::make_unique<SensorManager>(m_sensors, m_serial.get());

	//---------------------------------------------------------------------
	//ADD CONTROLS
	//---------------------------------------------------------------------

	//make the main sizer to organize things in the panel
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//this button is used to attempt connection with a microcontroller
	//using a serial connection
	m_connect_button = new wxButton(this, wxID_ANY, "Connect");
	
	//add the button to the sizer
	mainSizer -> Add(m_connect_button, 0, wxALIGN_CENTER | wxALL, 10);

	//set up the sizer with the added elements
	SetSizerAndFit(mainSizer);

	// Bind the Connect button to open the handshake dialog
	m_connect_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
		openConnectDialog();
	});

	// Bind the custom handshake success event to its handler
	//Bind(wxEVT_SERIAL_UPDATE, &ProjectPanel::onSerialUpdate, this);
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
	//stack allocate the dialog, and show it:
	HandshakeDialog handshaker(this, "Select serial port", m_serial.get());
	handshaker.ShowModal();
}

void ProjectPanel::onHandshakeSuccess(wxThreadEvent& evt)
{
	//evt is a thread event, and GetPayload<bool>
	//gets the result of the handshake (T = success)
    bool success = evt.GetPayload<bool>();
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


void ProjectPanel::startBackgroundPolling()
{
	/* \brief 	This function is used to open up a background thread
	 * 		whose only purpose is to read the incoming dataframes
	 * 		from the connected microcontroller, parse each frame,
	 * 		and assign sensor readings to all objects in the 
	 * 		class member m_sensors.
	 */

	if(!handshakeComplete){
		wxLogWarning("Cannot start polling: not connected");
		return;
	}

	if (running.exchange(true)){
		//polling already active (exchange sets running to 
		//true and returns the previous value)
		return;
	}
	
	//we start a new thread that runs in the background [this] 
	//lets the thread access the class's members as sensor
        ioThread = std::thread([this]() {
                        while (running.load()) //while running is true (load thread-safe)
                	{
				// we lock the serialComm so only one thread can access 
				// the hardware at a time which prevents
				// conflicts if multiple threads try to read/write at once
                               	std::lock_guard<std::mutex> lock(serialMutex);

			       	//read a dataframe from the microcontroller
				//Note that serialComm automatically populates the
				//vector of sensors with the appropriate readings.
				m_serial->readDataFrame(m_sensors);


       			       // small sleep to avoid overwhelming the arduino/esp32
         		std::this_thread::sleep_for(std::chrono::milliseconds(5));
      			}
        });
}
	
void ProjectPanel::stopBackgroundPolling()
{
	/* \brief the purpose of this method is to safely shut down the 
	 * background polling of sensor data from the microcontroller. 
	 *
	 *
	 */
        running.store(false);
	
	//we check if the background thread actually exists and is running
        if (ioThread.joinable())
        {
		//wait for it to finish its work safely before exiting it
                ioThread.join();
        }
}

//-----------------------------------------------------------------------------------------------------------
//TOOLBAR EVENTS
//-----------------------------------------------------------------------------------------------------------

void ProjectPanel::onSensors()
{
	//launch the SensorConfigDialog chain
	SensorConfigDialog dlg(this, "Sensor Configuration", m_serial.get(), m_sensorManager.get(), m_sensors);
	dlg.ShowModal();
}


