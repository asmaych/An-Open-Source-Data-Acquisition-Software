#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include "sensor/Sensor.h"
#include "CalibrationPoint.h"

/*
	DatabaseManager class is responsible for:
	    - opening a SQLite db
	    - creating tables if they don't exist
	    - saving and loading sensor templates
	    - creating and loading projects 
	    - save sensor config per project
	    - store experiment runs
	    - store continuous data frames
            - store collect on demand pointq
	    - store UI state
*/

class DatabaseManager
{
        private:
		//THis is a pointer to the SQLite db connection, and if the db isn't opened yet, the pointer will be null
		sqlite3* m_db;

        public:
                DatabaseManager();
		~DatabaseManager();

		//opens the db file located at path, returns true if opened successfully & false otherwise
		bool open(const std::string& path);

		//closes the db connection if its open which prevents memory leaks and file locking
		void close();

		//creates all required tables for the application using SQL create commands
		void createTables();

		//marks a sensor as explicitly saved by the user (user_saved = 1)
		bool markSensorAsSaved(const std::string& name);

		//saves a sensor by name & id to the db
		bool saveSensorTemplate(const std::string& name);

		//loads all sensor names from the db and puts them into a vector
		void loadSensorTemplates(std::vector<std::string>& names);

		//returns the sensor ID
		int getSensorID(const std::string& name);

		//creates a new project in the db and returns the id of the newly created project
		int createProject(const std::string& name);

		//load all saved projects
		bool loadProjects(std::vector<std::string>& projects);

		//get projectID by name
		int getProjectID(const std::string& name);

		bool saveProjectSampleRate(int id, float samplerate);

		bool loadProjectSampleRate(int project_id, int &sampleRate);

		//save sensors for the specific project
		bool saveProjectSensor(int projectId, int sensorId, int pin);

		//load sensors for a given project
		bool loadProjectSensors(int projectId, std::vector<std::pair<std::string, int>>& sensors);

		//create a run entry
		int createRun(int projectId);

		//create a frame
		int createFrame(int runId, double time);

		//save continuos data frame
		bool saveFrameValue(int frameId, int sensorId, double value);

		//save on demand collected point
		bool saveCollectPoint(int runId, int sensorId, double time, double value);

		//stores which UI panels where visible (graph/live display/collect on demand)
		void saveUIState(int projectId, bool graph, bool live, bool collect);

		//loads all runs for a project as (db_run_id, run_number) pairs
		bool loadProjectRuns(int projectId, std::vector<std::pair<int, int>>& runs);

		//loads all frames for a run and reconstructs per-sensor value vectores
		bool loadRunFrames(int runId, const std::vector<std::unique_ptr<Sensor>>& sensors, std::vector<std::pair<double, std::vector<double>>>& frames);

		//returns the db idof the most recent run for a project
		int getLastRunId(int projectId);

		//loads all collect points for a run as (time, sensor_id, value) tuples
		bool loadCollectPoints(int runId, std::vector<std::tuple<double,int,double>>& points);

		//load UI visibility state for a prject
		bool loadUIState(int projectId, bool& graph, bool& live, bool& collect);

		//saves or overwrites the global calibration for a sensor template
		bool saveGlobalCalibration(int sensorId, const std::string& type, const std::vector<CalibrationPoint>& points);

		//saves or replaces the calibration for one sensor instance in a project
		bool saveProjectCalibration(int projectId, int sensorId, int pin, const std::string& type, const std::vector<CalibrationPoint>& points);

		//loads the global calibration for a sensor template
		bool loadGlobalCalibration(int sensorId, std::string& type, std::vector<CalibrationPoint>& points);

		//loads the calibration for one sensor in a project
		bool loadProjectCalibration(int projectId, int sensorId, int pin, std::string& type, std::vector<CalibrationPoint>& points);

		//returns true if a global calibration exists for this sensor_id
		bool hasGlobalCalibration(int sensorId);

		//returns true if a project calibration exists for this specific sensor
		bool hasProjectCalibration(int projectId, int sensorId, int pin);

		//updates the pin for a sensor in project_sensors, called whenever the user changes a pin in hardwareConfirmDialog
		bool updateProjectSensorPin(int projectId, int sensorId, int oldPin, int newPin);

		//migrates the project_calibrations row from oldPin to newPin
		bool migrateCalibrationPin(int projectId, int sensorId, int oldPin, int newPin);

		//transaction helpers for DAQ performance cause without them the SQL is super slow
		void beginTransaction();
		void commitTransaction();
};


