#include "SensorDatabase.h"
#include <iostream>

SensorDatabase::SensorDatabase()
{
	// Constructor does nothing special.
        // Database is opened explicitly using open()
}

SensorDatabase::~SensorDatabase()
{
	// Make sure the database is closed when object is destroyed
        close();
}

/* open or create the sqlite db
	path: path to the sqlite file
	return true if successful, false otherwise
*/
bool SensorDatabase::open(const std::string& path)
{
	if(sqlite3_open(path.c_str(), &m_db) != SQLITE_OK){
		std::cerr << "Failed to open database: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

	//create tables if they do not exist
	createTables();

	return true;
}

//close the db connection
void SensorDatabase::close(){
	if(m_db){
		sqlite3_close(m_db); //close connection
		m_db = nullptr;
	}
}

void SensorDatabase::createTables(){
	/* create the sensors table if it doesn't exist
		columns:
			id -> unique db ID
			name -> sensor name as str
			enabled -> whether sensor is enabled (0 or 1(true))
	*/
	const char* sql = "CREATE TABLE IF NOT EXISTS sensors ("
                	   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                	   "name TEXT NOT NULL,"
                	   "pin INTEGER NOT NULL,"
                	   "selected INTEGER NOT NULL"
                	   ");";

	char* errMsg = nullptr;

	if(sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK){
		std::cerr << "Failed to create table: " << errMsg << std::endl;
        	sqlite3_free(errMsg);
    	}
}

void SensorDatabase::saveSensors(const std::vector<std::unique_ptr<Sensor>>& sensors){
	//do nothing if db isn't open
	if(!m_db)
		return;

	    char* errMsg = nullptr;

    // Remove all existing sensor records
    sqlite3_exec(m_db, "DELETE FROM sensors;", nullptr, nullptr, &errMsg);

    const char* sql =
        "INSERT INTO sensors (name, enabled) VALUES (?, ?);";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    // Bind each sensor's name and enabled state and insert
    for (const auto& sensor : sensors)
    {
        sqlite3_bind_text(stmt, 1, sensor->getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, sensor->isSelected() ? 1 : 0);

        sqlite3_step(stmt);  // execute insert
        sqlite3_reset(stmt); // reset for next row
    }

    sqlite3_finalize(stmt); // clean up statement
}

void SensorDatabase::loadSensors(std::vector<std::unique_ptr<Sensor>>& sensors){
	//do nothing if db not opened
	if(!m_db)
		return;

	//clear existing sensors in memory
	sensors.clear();

	const char* sql =
        "SELECT name, enabled FROM sensors;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    // Loop through each row and create a Sensor object
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // Read sensor name
        std::string name =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        // Read enabled flag (1 = true, 0 = false)
        bool enabled = sqlite3_column_int(stmt, 1);

        // Add sensor to the vector
        sensors.push_back(std::make_unique<Sensor>(name, enabled));
    }

    sqlite3_finalize(stmt); // clean up statement
}


