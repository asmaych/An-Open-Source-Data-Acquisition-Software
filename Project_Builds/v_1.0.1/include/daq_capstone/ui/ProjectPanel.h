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

		// Toolbar actions
		void startSelectedSensor();
		void stopSelectedSensor();
		void collectSelectedSensor();
		void graphSelectedSensor();
		void openConnectDialog();
		// A simple sensor Manager
		void openSensorPanel();
		

		
	private:
		//class attributes
		wxButton* m_connect_button = nullptr; //button to open handshake dialog
		wxListCtrl* m_sensorList = nullptr;
		std::unique_ptr<SerialComm> m_serial; //serialComm instance
		std::vector<std::unique_ptr<DataSession>> m_sessions; //DataSession per sensor
                std::vector<std::unique_ptr<Sensor>> sensors; //Sensor objects
		std::vector<std::unique_ptr<LiveDataWindow>> m_liveWindows; //Map sensor index -> LiveDataWindow
		
		//THreading for background
		std::thread ioThread;
		std::atomic<bool> running{false};
		bool handshakeComplete = false;
		std::mutex serialMutex;		

		//Add/Remove sensor
		void onAddSensor(wxCommandEvent& evt);
		void onRemoveSensor(wxCommandEvent& evt);
                void onSerialUpdate(wxThreadEvent& evt);
                void onHandshakeSuccess(wxThreadEvent& evt); 

		//helper functions
		int getSelectedSensorIndex() const;
		void startBackgroundPolling();
		void stopBackgroundPolling();
		
};


