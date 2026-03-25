#pragma once
#include <memory>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>
#include <thread>
#include <string>
#include <wx/splitter.h>
#include"controllers/SessionController.h"
#include "controllers/DataCollector.h"
#include "Theme.h"
#include "data/DataTableWindow.h"
#include "data/Run.h"
#include "data/GraphWindow.h"
#include "data/LiveDataWindow.h"
#include "db/DatabaseManager.h"
#include "Events.h"

/* ProjectPanel class represents one project tab, it contains:
   All sensors in a project, SerialComm instance for readings, Runs (continuous acquisition buffers)
   It provides: start/stop live readings, collect data, graph & export it, add/remove sensor
*/

class SerialComm;
class Sensor;
class SensorManager;
class DatabaseManager;
class Run;
class DataSession;
class LiveDataWindow;
class DataTableWindow;
class GraphWindow;
class HandshakeDialog;
class SessionController;
class MainFrame;

class ProjectPanel : public wxPanel
{
	public:
		ProjectPanel(wxWindow* parent, const wxString& title, DatabaseManager* db);
		~ProjectPanel() override;

		//Toolbar actions from Mainframe
		void toggleStartStop(); //start or stop a Run
		//Run lifecycle
                void startRun(); //creates a new run and starts collecting
                void stopRun(); //stops the current run
		void resetSessionData(); //clears all runs and reset live window
		void collectCurrentValues(); //collects on demand from the current run
		void graphSelectedSensor(wxCommandEvent& evt); //open graph window for selected sensor
		void onCollectNowGraphPoint(wxCommandEvent& evt); //graph the collected on demand point
		void resetTableWindow();
		void exportSessions(wxCommandEvent& evt); //export collected runs
		void updateLayout(); //to handle the new ui logic
		void onSensors(); //open sensor selection/management dialog
		void applyTheme(Theme theme); //black/light theme
		void adjustSampleRate(float rate) const;

		//MainFrame needs this to change toolbar state
		void setMainFrame(MainFrame* frame) {m_mainFrame = frame;}

		//event for new frame from serial
		void onNewDataFrame(const std::string& frame);

		//helper of export
		static wxString askSaveFile(wxWindow* parent);

		//ui helpers
                void openConnectDialog();

		//getter
		LiveDataWindow* getLiveWindow();
		DataTableWindow* getTableWindow();
		GraphWindow* getGraphWindow();
		std::shared_ptr<Run> getCurrentRun();
		bool isRunning() const;
		bool isConnected() const;
		const std::vector<std::unique_ptr<Sensor>>& getSensors() const;

		//parse csv string into vector<double>
                std::vector<double> parseSerialFrame(const std::string& frame);

		//used to update layout when buttons pressed
		void setGraphVisible(bool visible) {graphVisible = visible; }
		void setLiveVisible(bool visible) {liveVisible = visible; }
		void setCollectNowVisible(bool visible) {collectNowVisible = visible; }

		//functions to save project in db
		void setProjectId(int id) {m_projectId = id;}
		int getProjectID() const;
		void setSaveProject(bool value) { m_saveProjectToDB = value;}
		void loadProjectFromDatabase();
		bool shouldSaveProject() const; //used to know if project should persist

		//Mainframe needs access to sensors for selection dialogs
		std::vector<std::unique_ptr<Sensor>> m_sensors;

		//putting this in public so that objects can see if it has a connected controller
		bool handshakeComplete = false;

	private:
		//splitter for live graph (top) live table (bottom)
		std::unique_ptr<wxSplitterWindow> m_splitter;
		std::unique_ptr<wxSplitterWindow> m_bottom_splitter; //for splitting the buttom space when collect now is used

		//graph helpers
		void graphTable(DataTableWindow*); //graph collect on demand table
		void graphRun(const std::shared_ptr<Run>& run); //graph a full run

		//export helpers
		void exportRun(const std::shared_ptr<Run>& run, const wxString& path);
		void exportTable(DataTableWindow* table, const wxString& path);
		void exportGraph(GraphWindow* graph, const wxString& path);

		wxListCtrl* m_sensorList{}; //display all available sensors
		wxButton* m_connect_button{}; //button to open handshake dialog

		//hardware & control
		std::unique_ptr<SerialComm> m_serial; //SerialComm instance
		std::unique_ptr<SensorManager> m_sensorManager;
		std::unique_ptr<SessionController> m_controller;

		//live display & tables
		std::unique_ptr<LiveDataWindow> m_liveWindow; //single embedded live window
		std::unique_ptr<DataTableWindow> m_tableWindow; //single collect on demand window collection
		std::unique_ptr<DataTableWindow> m_continuousTableWindow; //table for run
		std::unique_ptr<GraphWindow> m_graphWindow; //current graph window

		//state flags
		bool graphVisible = false; //assigned to true cause when pressing start both graph and collect continuous are displayed
		bool liveVisible = false;
		bool collectNowVisible = false; //depends on the user

		//all recorded runs for this project
		std::vector<std::shared_ptr<Run>> m_runs;

		//currently active run
		std::shared_ptr<Run> m_currentRun;

		//mainFrame
		MainFrame* m_mainFrame = nullptr;

		//current theme by default
		Theme m_currentTheme;

		//Absolute start time of the active run in seconds
		double m_runStartTime = 0.0;

		//Events sent from background serial thread
                void onHandshakeSuccess(wxThreadEvent& evt);
                void onSerialUpdate(wxThreadEvent& evt);

		//State flags
		bool m_isRunning = false;

		//database lives as long as the project
		DatabaseManager* m_DB = nullptr;

		//project id
		int m_projectId = -1;

		//do we save/not save project to db
		bool m_saveProjectToDB = false;

		//project name
		wxString m_projectName;

		//run id for db
		int m_currentRunId = -1;
};
