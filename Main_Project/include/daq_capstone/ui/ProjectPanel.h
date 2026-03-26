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

/**
 * @brief This is the GUI object that wraps around all data and configurations of a project.
 *
 * When a user sets up or loads a project, this is the main class around which all activities revolve. In terms of
 * ownership, nearly every other class can be traced back to this. Most in-project events are handled within this class,
 * which is something that we are looking at reevaluating. In the future, we may delegate more event handlers to more
 * specialized classes, but it is not expected that the class ownership hierarchy will change.
 */
class ProjectPanel : public wxPanel
{
	public:
		/**
		* @brief Used to initialize all wxWidgets controls and class attributes, and bind event handlers to the controls.
		*
		* @param parent Wxwidgets enforces strict inheritance of other wxwidgets objects. This one inherits from MainFrame
		* @param title This is the name that the user is prompted to enter in MainFrame::onNewProject()
		*/
		ProjectPanel(wxWindow* parent, const wxString& title, DatabaseManager* db);

		/**
		 *	@brief	Closes the database, shuts down the background polling thread, and resets the microcontroller
		 *
		 * When run, the opened database for the project will be closed. A reset command is sent to the microcontroller
		 * using the SerialComm instance m_serial. If a thread has been started to begin collecting dataframes from the
		 * serial input buffer, a command is sent to the SessionController instance m_controller to shut down the thread
		 *
		 * @note This method is marked as override because it adds functionality not present in the wxWindow class it
		 * inherits from.
		 */
		~ProjectPanel() override;

		//Toolbar actions from Mainframe
		/**
		 * @brief Toggle function used to control sensor data recording sessions.
		 *
		 * This method is called when the user clicks the Start/Stop toggle button located in Toolbar. When called, it
		 * checks to see if a run is already in session, and if so, stops it. If there is no active running recording
		 * session, it will create a new one.
		 *
		 * @note If a new recording session is created, it will do so without destroying any previous runs. Unless the
		 * resetSessionData method is invoked, all runs of data will be retained by default, so that multiple runs can
		 * be recorded, compared, and/or exported by the user.
		 */
		void toggleStartStop(); //start or stop a Run

		//Run lifecycle
		/**
		 * @brief Creates new run of data into which the polling thread begins to write sensor readings with timestamps
		 *
		 * This method creates a new run of data, and points m_currentRun to it. When a dataframe is read and parsed,
		 * readings will be sent to this session. It sets a boolean flag m_isRunning to true, so that when the polling
		 * thread invokes onNewDataFrame the method knows whether it is allowed to write the values to m_currentRun
		 */
		void startRun(); //creates a new run and starts collecting

		/**
		 * @brief Shuts down current run of data. Recorded data is retained until user either resets, or exits
		 *
		 * This method simply flips the boolean flag m_isRunning to false, which prevents onNewDataFrame() from writing
		 * incoming readings to m_currentRun.
		 */
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
		void adjustSampleRate(float rate);

		SensorManager * getSensorManager() const;

		float getSampleRate() const;

		//MainFrame needs this to change toolbar state
		void setMainFrame(MainFrame* frame) {m_mainFrame = frame;}

		//event for new frame from serial
		void onNewDataFrame();

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

		float m_sampleRate{50};
};
