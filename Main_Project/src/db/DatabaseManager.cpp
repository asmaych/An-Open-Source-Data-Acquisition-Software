#include "DatabaseManager.h"
#include <iostream>
#include <memory>
#include <map>

/*
	Constructor which runs when we create a DatabaseManager object, we initialize m_db to nullptr to indicate that the db is not opened yet
*/

DatabaseManager::DatabaseManager()
{
	m_db = nullptr;
}


/*
	Destructor which runs automatically when the object is destroyed, where we call close() to ensure the database connection is 100% closed before the program exits
*/
DatabaseManager::~DatabaseManager()
{
	close();
}


/*
	opens the db file located at "path", returns true if successfully opened, false + msg otherwise
*/
bool DatabaseManager::open(const std::string& path)
{
    	/*
        	sqlite3_open() command opens a SQLite db file, where:
        	path.c_str() converts std::string to C string
        	&m_db the connection pointer will be stored here
    	*/
    	if(sqlite3_open(path.c_str(), &m_db) != SQLITE_OK){
        	//if it fails we display an error message
        	std::cerr << "Failed to open database: " << sqlite3_errmsg(m_db) << std::endl;

        	return false;
    	}

    	std::cout << "Database opened successfully\n";

	//enable foreign key enforcement in SQLite
	sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    	//after opening the db we create tables if they do not exist
    	createTables();

    	return true;
}


/*
	closes the database connection if it is open which prevents memory leaks and file locking
*/
void DatabaseManager::close()
{
    	//only close the db if it is open
    	if(m_db){
        	//sqlite3_close() releases the database connection
        	sqlite3_close(m_db);

        	//we set pointer back to nullptr to avoid accidentally using it
	        m_db = nullptr;
    	}
}


/*
	 creates all required tables for the application
*/
void DatabaseManager::createTables()
{
    	//here I used a RAW STRING (R"( ... )") so I can write multi-line SQL easily

    	const char* sql = R"(

    		CREATE TABLE IF NOT EXISTS sensors(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		name TEXT NOT NULL UNIQUE,
			user_saved INTEGER NOT NULL DEFAULT 0
		);

    		CREATE TABLE IF NOT EXISTS projects(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		name TEXT NOT NULL UNIQUE,
        		created_at TEXT,
			sample_rate INTEGER NOT NULL DEFAULT 50
    		);

    		CREATE TABLE IF NOT EXISTS project_sensors(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		project_id INTEGER NOT NULL,
        		sensor_id INTEGER NOT NULL,
        		pin INTEGER NOT NULL,

			UNIQUE(project_id, pin),

			FOREIGN KEY(project_id) REFERENCES projects(id),
			FOREIGN KEY(sensor_id) REFERENCES sensors(id)
    		);

    		CREATE TABLE IF NOT EXISTS runs(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		project_id INTEGER NOT NULL,
        		start_time TEXT,
        		end_time TEXT,

			FOREIGN KEY(project_id) REFERENCES projects(id)
    		);

    		CREATE TABLE IF NOT EXISTS frames(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		run_id INTEGER NOT NULL,
        		time REAL,
			FOREIGN KEY(run_id) REFERENCES runs(id)
    		);

		CREATE TABLE IF NOT EXISTS frame_values(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
    			frame_id INTEGER NOT NULL,
    			sensor_id INTEGER NOT NULL,
    			value REAL,

			FOREIGN KEY(frame_id) REFERENCES frames(id),
        		FOREIGN KEY(sensor_id) REFERENCES sensors(id)
		);

    		CREATE TABLE IF NOT EXISTS collect_points(
        		id INTEGER PRIMARY KEY AUTOINCREMENT,
        		run_id INTEGER NOT NULL,
        		sensor_id INTEGER NOT NULL,
			time REAL,
           		value REAL,

			FOREIGN KEY(run_id) REFERENCES runs(id),
        		FOREIGN KEY(sensor_id) REFERENCES sensors(id)
    		);

    		CREATE TABLE IF NOT EXISTS ui_state(
        		project_id INTEGER PRIMARY KEY,
        		graph_visible INTEGER,
        		live_visible INTEGER,
        		collect_visible INTEGER,

			FOREIGN KEY(project_id) REFERENCES projects(id)
    		);

    	)";

	//errMsg will store any error message returned by SQLite
    	char* errMsg = nullptr;

    	/*	sqlite3_exec() executes the SQL commands (db, sql_command, callback, callback_argument, error_msg) 
		which means (the opened db connection, the SQL commands we want to run, no callback function, no data passed to callback, where SQLite store the error msg)
    	*/
	if(sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK){

		std::cerr << "Failed to create tables: " << sqlite3_errmsg(m_db) << std::endl;

	        sqlite3_free(errMsg);
    	}
}


/*
	marks a sensor as explicitly saved by the user (user_saved = 1)
*/
bool DatabaseManager::markSensorAsSaved(const std::string& name)
{
    	//sensors with user_saved = 0 are invisible to the catalogue and exist only to satisfy the foreign key in project_sensors
    	const char* sql = "UPDATE sensors SET user_saved = 1 WHERE name = ?;";

    	sqlite3_stmt* stmt = nullptr;

    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "markSensorAsSaved prepare error: " << sqlite3_errmsg(m_db) << "\n";
        	return false;
    	}

    	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    	bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    	sqlite3_finalize(stmt);
    	return ok;
}

/*
	saves sensors to db by name
*/
bool DatabaseManager::saveSensorTemplate(const std::string& name)
{
    	//SQL statement with a placeholder (?), where we will bind the value later
    	//iam using ignore to prevents errors since the name was declared as unique
	const char* sql = "INSERT OR IGNORE INTO sensors(name) VALUES(?);";

    	sqlite3_stmt* stmt = nullptr;

    	//sqlite3_prepare_v2() compiles the SQL query into a prepared statement which is safer and faster than building raw SQL strings
        if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
    	    	std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

    	//bind the sensor name to parameter 1
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    	//execute the statement
    	if(sqlite3_step(stmt) != SQLITE_DONE){
        	sqlite3_finalize(stmt);
        	return false;
    	}

    	//sqlite3_finalize() frees the prepared statement memory
        sqlite3_finalize(stmt);

    	return true;
}


/*
	loads all sensor names from the db and puts them into a vector that is passed by reference so the function can modify it directly
*/
void DatabaseManager::loadSensorTemplates(std::vector<std::string>& names)
{
    	//clear the vector so we don't append duplicates
    	names.clear();

    	const char* sql = "SELECT name FROM sensors WHERE user_saved = 1;";

    	sqlite3_stmt* stmt = nullptr;

    	//we prepare the SQL query
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return;
	}

    	//sqlite3_step() returns rows one by one
	while(sqlite3_step(stmt) == SQLITE_ROW){
        	/*
            		read the column value -> sqlite3_column_text returns a const unsigned char*, so we cast it to const char*
        	*/
        	std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));

		if(name.empty())
			continue;

        	//add the sensor name to the vector
        	names.push_back(name);
    	}

    	//free the statement memory
	sqlite3_finalize(stmt);
}


/*
	gets the id created for the sensor in the template
*/
int DatabaseManager::getSensorID(const std::string& name)
{
	const char* sql = "SELECT id FROM sensors WHERE name=?;";

        sqlite3_stmt* stmt = nullptr;

        if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
                return -1;
	}

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        int id = -1;

        if(sqlite3_step(stmt) == SQLITE_ROW)
                id = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);

        return id;
}


/*
	creates a new project in the db where each project has a name & a creation timestamp, and SQL automatically generates a unique ID of the newly created project, if smth fails it returns -1
*/
int DatabaseManager::createProject(const std::string& name)
{
	//SQL query to insert a new project by name and creation time
	const char* sql = "INSERT INTO projects (name, created_at) VALUES (?, datetime('now'));";

    	sqlite3_stmt* stmt = nullptr;

	//sqlite3_prepare_v2() compiles the SQL query into a format SQLite can execute
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return -1;
	}

	//this attaches the project name to the SQL placeholder (?)
    	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    	if(sqlite3_step(stmt) != SQLITE_DONE){
        	//if smth failed during execution, we must free the statement before exiting
		sqlite3_finalize(stmt);
        	return -1;
    	}

	//sqlite3_finalize() releases the memory used by the prepared statement, which is extremely important to avoid memory leaks&db locks
    	sqlite3_finalize(stmt);

    	//return ID of the inserted project
	//explicit cast from sqlite3_int64 to int
    	return (int)sqlite3_last_insert_rowid(m_db);
}


/*
	gets all saved projects
*/
bool DatabaseManager::loadProjects(std::vector<std::string>& projects)
{
	//if no db is created, no project can be loaded
	if(!m_db)
		return false;

	//clear the vector to avoid appending duplicates
	projects.clear();

	//SQL query to get all project names sorted from oldest to newest according to the creating time
	const char* sql = "SELECT name FROM projects ORDER BY created_at ASC;";
	sqlite3_stmt* stmt = nullptr;

	//prepare the SQL statement for execution
	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

	//execute the statement and fetch each row one by one
	while(sqlite3_step(stmt) == SQLITE_ROW){
		//get the project name from column 0
		std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

		//add the project name to the vector
		projects.push_back(name);
	}

	//free the memory used by the prepared statement
	sqlite3_finalize(stmt);

	return true;
}


/*
	finds the database ID for a project given its name
*/
int DatabaseManager::getProjectID(const std::string& name)
{
    	//check if the database is open
    	if(!m_db)
		return -1;

    	//SQL query to select the project ID where the name matches
    	const char* sql = "SELECT id FROM projects WHERE name = ?;";
    	sqlite3_stmt* stmt = nullptr;

    	//prepare the SQL statement
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return -1;
	}

    	//bind the project name to the first parameter (?)
    	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    	int id = -1; //default ID if not found

   	//execute the statement; check if a row is returned
    	if(sqlite3_step(stmt) == SQLITE_ROW){
        	//get the integer ID from column 0
        	id = sqlite3_column_int(stmt, 0);
    	}

    	//free the statement memory
    	sqlite3_finalize(stmt);

    	return id;
}

bool DatabaseManager::saveProjectSampleRate(int id, float samplerate) {
	if (!m_db) {
		std::cerr << "Database not initialized.\n";
		return false;
	}

	// Use a parameterized query safely
	const char* sql = "UPDATE projects SET sample_rate = ? WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Prepare failed: " << sqlite3_errmsg(m_db) << "\n";
		return false;
	}

	// Bind sampleRate
	if (sqlite3_bind_double(stmt, 1, samplerate) != SQLITE_OK) {
		std::cerr << "Bind sampleRate failed: " << sqlite3_errmsg(m_db) << "\n";
		sqlite3_finalize(stmt);
		return false;
	}

	// Bind project_id
	if (sqlite3_bind_int(stmt, 2, id) != SQLITE_OK) {
		std::cerr << "Bind id failed: " << sqlite3_errmsg(m_db) << "\n";
		sqlite3_finalize(stmt);
		return false;
	}

	// Execute
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		std::cerr << "Execution failed: " << sqlite3_errmsg(m_db) << "\n";
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}


bool DatabaseManager::loadProjectSampleRate(int project_id, int& sampleRate)
{
	if (!m_db) {
		std::cerr << "Database not initialized.\n";
		return false;
	}

	const char* sql = "SELECT sample_rate FROM projects WHERE id = ?;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Prepare failed: " << sqlite3_errmsg(m_db) << "\n";
		return false;
	}

	// Bind project_id
	if (sqlite3_bind_int(stmt, 1, project_id) != SQLITE_OK) {
		std::cerr << "Bind project_id failed: " << sqlite3_errmsg(m_db) << "\n";
		sqlite3_finalize(stmt);
		return false;
	}

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		sampleRate = static_cast<float>(sqlite3_column_double(stmt, 0));
	} else if (rc == SQLITE_DONE) {
		std::cerr << "No project found with project_id = " << project_id << "\n";
		sqlite3_finalize(stmt);
		return false;
	} else {
		std::cerr << "Execution failed: " << sqlite3_errmsg(m_db) << "\n";
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}

/*
	saves a sensor used in a specific project
*/
bool DatabaseManager::saveProjectSensor(int projectId, int sensorId, int pin)
{
    	const char* sql = "INSERT OR IGNORE INTO project_sensors (project_id, sensor_id, pin) " "VALUES (?, ?, ?);";

    	sqlite3_stmt* stmt = nullptr;

    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

    	sqlite3_bind_int(stmt, 1, projectId);
    	sqlite3_bind_int(stmt, 2, sensorId);
    	sqlite3_bind_int(stmt, 3, pin);

    	if(sqlite3_step(stmt) != SQLITE_DONE){
        	sqlite3_finalize(stmt);
        	return false;
    	}

    	sqlite3_finalize(stmt);

    	return true;
}


/*
	loads all sensors assigned to a project (true is all successfully uploaded, false otherwise)
*/
bool DatabaseManager::loadProjectSensors(int projectId, std::vector<std::pair<std::string,int>>& sensors)
{
    	//checks if database is open
    	if(!m_db) 
		return false;

    	//clears the vector to avoid duplicates
    	sensors.clear();

    	//SQL query to select sensor id and pin for the project
	//i joined sensors table to retrieve sensor names
    	const char* sql = "SELECT sensors.name, project_sensors.pin "
			  "FROM project_sensors "
			  "JOIN sensors ON sensors.id = project_sensors.sensor_id "
			  "WHERE project_sensors.project_id = ?;";

	sqlite3_stmt* stmt = nullptr;

    	//prepare the statement
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return false;
	}

    	//bind project ID to the query
    	sqlite3_bind_int(stmt, 1, projectId);

    	//fetch each sensor row by row
    	while(sqlite3_step(stmt) == SQLITE_ROW){
        	//get sensor name from column 0
        	std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        	//get pin number from column 1
        	int pin = sqlite3_column_int(stmt, 1);

        	//add the sensor to the vector
        	sensors.push_back({name, pin});
    	}

    	//release statement memory
    	sqlite3_finalize(stmt);

    	return true;
}


/*
	creates a run entry for the specific project
*/
int DatabaseManager::createRun(int projectId)
{
    	const char* sql = "INSERT INTO runs (project_id, start_time) " "VALUES (?, datetime('now'));";

    	sqlite3_stmt* stmt = nullptr;

    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return -1;
	}

    	sqlite3_bind_int(stmt, 1, projectId);

    	if(sqlite3_step(stmt) != SQLITE_DONE){
	        sqlite3_finalize(stmt);
        	return -1;
    	}

    	sqlite3_finalize(stmt);

    	return sqlite3_last_insert_rowid(m_db);
}


/*
	creates frames
*/
int DatabaseManager::createFrame(int runId,double time)
{

	const char* sql = "INSERT INTO frames(run_id,time) VALUES(?,?);";

	sqlite3_stmt* stmt=nullptr;

	if(sqlite3_prepare_v2(m_db,sql,-1,&stmt,nullptr)!=SQLITE_OK){
    		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return -1;
	}

	sqlite3_bind_int(stmt,1,runId);
	sqlite3_bind_double(stmt,2,time);

	if(sqlite3_step(stmt)!=SQLITE_DONE){
    		sqlite3_finalize(stmt);
    		return -1;
	}

	sqlite3_finalize(stmt);

	return sqlite3_last_insert_rowid(m_db);
}


/*
	saves continuous collection of data
*/
bool DatabaseManager::saveFrameValue(int frameId, int sensorId, double value)
{
	const char* sql = "INSERT INTO frame_values(frame_id,sensor_id,value)""VALUES(?,?,?);";

	sqlite3_stmt* stmt=nullptr;

	if(sqlite3_prepare_v2(m_db,sql,-1,&stmt,nullptr)!=SQLITE_OK){
    		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt,1,frameId);
	sqlite3_bind_int(stmt,2,sensorId);
	sqlite3_bind_double(stmt,3,value);

	if(sqlite3_step(stmt)!=SQLITE_DONE){
    		sqlite3_finalize(stmt);
    		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}


/*
	saves a manually collected point
*/
bool DatabaseManager::saveCollectPoint(int runId, int sensorId, double time, double value)
{
    	const char* sql = "INSERT INTO collect_points(run_id, sensor_id, time, value) " "VALUES(?,?,?,?);";

    	sqlite3_stmt* stmt = nullptr;

    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return false;
    	}

    	sqlite3_bind_int(stmt,1,runId);
    	sqlite3_bind_int(stmt,2,sensorId);
    	sqlite3_bind_double(stmt,3,time);
    	sqlite3_bind_double(stmt,4,value);

    	if(sqlite3_step(stmt) != SQLITE_DONE){
        	sqlite3_finalize(stmt);
        	return false;
    	}

    	sqlite3_finalize(stmt);

    	return true;
}


/*
	stores which UI panels are visible when the project was saved
*/
void DatabaseManager::saveUIState(int projectId, bool graph, bool live, bool collect)
{
	//SQL command to insert UI state values into the db
	//here im using replace instead of insert to avoid duplicates
    	const char* sql = "REPLACE INTO ui_state " "(project_id, graph_visible, live_visible, collect_visible) " "VALUES (?, ?, ?, ?);";

	sqlite3_stmt* stmt = nullptr;

	//sqlite3_prepare_v2() compiles the SQL query into a format SQLite can execute
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return;
	}

	//bind all of the different values to their placeholders
    	sqlite3_bind_int(stmt, 1, projectId);
	// true -> 1 while false -> 0
    	sqlite3_bind_int(stmt, 2, graph ? 1 : 0);
    	sqlite3_bind_int(stmt, 3, live ? 1 : 0);
    	sqlite3_bind_int(stmt, 4, collect ? 1 : 0);

	//execute the SQL command
    	sqlite3_step(stmt);

	//release the prepared statement memory
    	sqlite3_finalize(stmt);
}


/*
	loads all runs for a project as (db_run_id, run_number) pairs
*/
bool DatabaseManager::loadProjectRuns(int projectId, std::vector<std::pair<int,int>>& runs)
{
    	runs.clear();

    	if(!m_db)
		return false;

    	//select all runs for this project ordered by start_time, we generate the run_number as the row position used
    	const char* sql = "SELECT id FROM runs WHERE project_id = ? ORDER BY start_time ASC;";

    	sqlite3_stmt* stmt = nullptr;

	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "loadProjectRuns prepare error: " << sqlite3_errmsg(m_db) << "\n";
        	return false;
    	}

	sqlite3_bind_int(stmt, 1, projectId);

    	int runNumber = 1;
    	while(sqlite3_step(stmt) == SQLITE_ROW){
        	int dbRunId = sqlite3_column_int(stmt, 0);
        	runs.push_back({dbRunId, runNumber++});
    	}

    	sqlite3_finalize(stmt);
    	return true;
}


/*
	loads all frames for a run and reconstructs per-sensor value vectors
*/
bool DatabaseManager::loadRunFrames(int runId, const std::vector<std::unique_ptr<Sensor>>& sensors, std::vector<std::pair<double, std::vector<double>>>& frames)
{
    	frames.clear();

    	if(!m_db)
		return false;

    	//build a map of sensor_id -> index in the sensors vector so we can place each value in the correct column position which must 
	//match the order sensors appear in m_sensors
    	std::map<int, size_t> sensorIndexMap;
    	for(size_t i = 0; i < sensors.size(); ++i){
        	int sid = getSensorID(sensors[i] -> getName());
        	if(sid >= 0)
            		sensorIndexMap[sid] = i;
    	}

    	size_t sensorCount = sensors.size();

    	//load all frames for this run ordered by time
    	const char* frameSql = "SELECT id, time FROM frames WHERE run_id = ? ORDER BY time ASC;";

    	sqlite3_stmt* frameStmt = nullptr;
    	if(sqlite3_prepare_v2(m_db, frameSql, -1, &frameStmt, nullptr) != SQLITE_OK){
        	std::cerr << "loadRunFrames frame prepare error: " << sqlite3_errmsg(m_db) << "\n";
        	return false;
    	}

    	sqlite3_bind_int(frameStmt, 1, runId);

    	//for each frame, load its sensor values
    	const char* valueSql = "SELECT sensor_id, value FROM frame_values WHERE frame_id = ?;";

    	while(sqlite3_step(frameStmt) == SQLITE_ROW){
        	int frameId = sqlite3_column_int(frameStmt, 0);
        	double t = sqlite3_column_double(frameStmt, 1);

        	//start with zeros for all sensors — any sensor not found in frame_values keeps its zero to handle missing/partial frames in case a sensor is removed in the mid of an exp
        	std::vector<double> values(sensorCount, 0.0);

        	sqlite3_stmt* valStmt = nullptr;
        	if(sqlite3_prepare_v2(m_db, valueSql, -1, &valStmt, nullptr) == SQLITE_OK){
            		sqlite3_bind_int(valStmt, 1, frameId);

            		while(sqlite3_step(valStmt) == SQLITE_ROW){
                		int sensorId = sqlite3_column_int(valStmt, 0);
                		double value = sqlite3_column_double(valStmt, 1);

                		//place value at the correct sensor index
                		auto it = sensorIndexMap.find(sensorId);
                		if(it != sensorIndexMap.end())
                    			values[it->second] = value;
            		}
            		sqlite3_finalize(valStmt);
        	}

        	frames.push_back({t, values});
    	}

    	sqlite3_finalize(frameStmt);
    	return true;
}

/*
	loads all collect points for a run as (time, sensor_id, value) tuples
*/
bool DatabaseManager::loadCollectPoints(int runId, std::vector<std::tuple<double,int,double>>& points)
{
    	points.clear();

    	if(!m_db || runId < 0)
        	return false;

    	const char* sql = "SELECT time, sensor_id, value FROM collect_points " "WHERE run_id = ? ORDER BY time ASC;";

    	sqlite3_stmt* stmt = nullptr;
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "loadCollectPoints error: " << sqlite3_errmsg(m_db) << "\n";
        	return false;
    	}

    	sqlite3_bind_int(stmt, 1, runId);

    	while(sqlite3_step(stmt) == SQLITE_ROW){
        	double t = sqlite3_column_double(stmt, 0);
        	int sensorId = sqlite3_column_int(stmt, 1);
        	double value = sqlite3_column_double(stmt, 2);
        	points.emplace_back(t, sensorId, value);
    	}

    	sqlite3_finalize(stmt);
    	return true;
}


/*
	loads which UI windows were visible for a project
*/
bool DatabaseManager::loadUIState(int projectId, bool& graph, bool& live, bool& collect)
{
    	//check if database is open
    	if(!m_db) 
		return false;

    	//SQL query to get visibility for the project
    	const char* sql = "SELECT graph_visible, live_visible, collect_visible FROM ui_state WHERE project_id = ?;";
    	sqlite3_stmt* stmt = nullptr;

    	//prepare the statement
    	if(sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return false;
	}

    	//bind project ID to the query
    	sqlite3_bind_int(stmt, 1, projectId);

    	//execute the query
    	if(sqlite3_step(stmt) == SQLITE_ROW){
        	//convert integer 0/1 from database to boolean values
        	graph = sqlite3_column_int(stmt, 0) != 0;
        	live = sqlite3_column_int(stmt, 1) != 0;
        	collect = sqlite3_column_int(stmt, 2) != 0;
    	} else {
        	//if no UI state is saved, apply defaults
        	graph = true;
        	live = true;
        	collect = false;
		sqlite3_finalize(stmt);
		return false;	
    	}

    	//free statement memory
    	sqlite3_finalize(stmt);

    	return true;
}


/*
	transaction helpers for high-speed DAQ writes
*/

void DatabaseManager::beginTransaction()
{
    	sqlite3_exec(m_db,"BEGIN TRANSACTION;",nullptr,nullptr,nullptr);
}

void DatabaseManager::commitTransaction()
{
    	sqlite3_exec(m_db,"COMMIT;",nullptr,nullptr,nullptr);
}
