#pragma once
#include <memory>
#include <wx/wx.h>
#include "SerialComm.h"
#include <wx/listctrl.h>
#include <vector>
#include <thread>
#include <atomic>
#include"controllers/SessionController.h"
#include "controllers/DataCollector.h"
#include "controllers/ExportManager.h"
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "Events.h"
#include "data/LiveDataWindow.h"
#include "data/DataTableWindow.h"
#include "data/GraphWindow.h"
#include "HandshakeDialog.h"
#include "SensorManager.h"
#include "SensorConfigDialog.h"
#include "MainFrame.h"

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
class SessionController;

class ProjectPanel : public wxPanel
{
	public:
		ProjectPanel(wxWindow* parent, const wxString& title);
		~ProjectPanel() override;

		void toggleStartStop(); //start or stop polling
		void resetSessionData(); //reset live window and session data
		void collectContinuous(); //collects continuously selected sensors (append)
		void collectCurrentValues(); //collects last value from all selected sensors
		void graphSelectedSensor(); //open graph window for selected sensor
		void openSensorPanel(); //open sensorConfig dialog
		void exportSessions(); //export collected session(s)
		void onSensors(); //open sensor selection/management dialog
		void onNewDataFrame(const std::string& frame);

		void setMainFrame(MainFrame* frame) {m_mainFrame = frame;}

	private:

		wxListCtrl* m_sensorList = nullptr; //display all available sensors
		wxButton* m_connect_button = nullptr; //button to open handshake dialog

		std::unique_ptr<SerialComm> m_serial; //SerialComm instance

		std::vector<std::unique_ptr<DataSession>> m_sessions; //a vector of data sessions
		std::unique_ptr<LiveDataWindow> m_liveWindow; //single embedded live window
		std::vector<GraphWindow*> m_graphWindows; //graph windows
		DataTableWindow* m_tableWindow = nullptr; //single collect window per project for last-value collection
		DataTableWindow* m_continuousTableWindow = nullptr; //single collect window for continuous collection

		std::unique_ptr<DataCollector> m_collector;
		std::shared_ptr<SessionController> m_controller;
		std::unique_ptr<SensorManager> m_sensorManager; //sensorManager object
                std::vector<std::unique_ptr<Sensor>> m_sensors; //all added Sensor objects  
		std::vector<Sensor*> m_activeSensors; //sensors selected for reading
		std::vector<int> m_selectedSensorIndexes; //stores user-selected sensor indexes for collection
		std::vector<size_t> m_lastContinuousRow;
		std::vector<int> m_selectedContinuousIndexes; 
		std::vector<int> m_selectedCurrentIndexes;


		MainFrame* m_mainFrame = nullptr;

		bool handshakeComplete = false;
		bool isRunning = false;
		std::atomic <bool> m_running {false};
		std::thread m_runningThread;

		int getSelectedSensorIndex() const;
		void startPollingIfNeeded();
		void stopPollingIfNeeded();
		void openConnectDialog();
		void onSerialUpdate(wxThreadEvent& evt);
		void onHandshakeSuccess(wxThreadEvent& evt);
		void refreshSensorList();

		void ensureSessionForSensor(size_t index);
};


