#pragma once
#include <memory>
#include <wx/wx.h>
#include "SerialComm.h"
#include <wx/listctrl.h>
#include <vector>
#include <thread>
#include <atomic>
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "Events.h"
#include "data/LiveDataWindow.h"
#include "data/DataTableWindow.h"
#include "data/GraphWindow.h"
#include "HandshakeDialog.h"
#include "SensorManager.h"
#include "SensorConfigDialog.h"


/* ProjectPanel class represents one project tab, it contains:
   List of sensors, SerialComm instance for readings, DataSessions for each sensor
   It provides: start/stop live readings, collect data, graph it, add/remove sensor
*/
class SerialComm;
class Sensor;
class DataSession;
class LiveDataWindow;
class DataTableWindow;
class GraphWindow;
class HandshakeDialog;

class ProjectPanel : public wxPanel
{
	public:
		ProjectPanel(wxWindow* parent, const wxString& title);
		~ProjectPanel();

		void openConnectDialog();
		// A simple sensor Manager
		void openSensorConfig();

		//helper function to open sensorconfigdialog
		void onSensors();
		

		
	private:
		//-------------------------------------------------------------------------------------------
		//class attributes
		//-------------------------------------------------------------------------------------------
		wxButton* m_connect_button = nullptr; 		//Button to open handshake dialog
		std::unique_ptr<SerialComm> m_serial; 		//SerialComm instance
		std::unique_ptr<SensorManager> m_sensorManager;	//sensorManager object
		std::vector<std::unique_ptr<Sensor>> m_sensors;	//vector to store Sensor objects		
		std::thread ioThread;				//A background thread for polling sensors
		std::atomic<bool> running{false};		//this ensures thread-safety of ioThread
		bool handshakeComplete = false;			//helper bool for handshake
		std::mutex serialMutex;				//ensures thread-safety of SerialComm object	

		//-------------------------------------------------------------------------------------------
		//Event handlers
		//-------------------------------------------------------------------------------------------
                void onHandshakeSuccess(wxThreadEvent& evt); 	//handles successful handshake

		//-------------------------------------------------------------------------------------------
		//helper functions
		//-------------------------------------------------------------------------------------------
		void startBackgroundPolling();			//opens a background thread to poll sensors
		void stopBackgroundPolling();			//closes the background thread
		
};


