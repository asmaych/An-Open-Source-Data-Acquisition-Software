#include "SensorDatabase.h"
#include <iostream>
#define PROJECT_ROOT "../"

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
	if(sqlite3_open(PROJECT_ROOT "sensors.db", &m_db) != SQLITE_OK){
		std::cerr << "Failed to open database: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

	std::cout << "Database opened successfully.\n";

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
			no pin because pin numbers depend on the user's hardware & how they wired the sensor
		therefore:
			database -> catalog of available sensor types
			project -> actual sensors with assigned pins
	*/

	std::cout << "Creating sensors table...\n";
	const char* sql = "CREATE TABLE IF NOT EXISTS sensors ("
                	   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                	   "name TEXT NOT NULL UNIQUE"
                	   ");";

	char* errMsg = nullptr;

	if(sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK){
		std::cerr << "Failed to create table: " << errMsg << std::endl;
        	sqlite3_free(errMsg);
    	}
}

bool SensorDatabase::saveSensors(const std::string& name){
	/* saves a sensor template into the db
		a template that contains only the name for now, calibration after.
		in other words, a table called sensors with a columns where one of them is called name just as an Excel file.
	*/

	//do nothing if db isn't open (m_db is a pointer to the db connection)
	if(!m_db)
		return false;

	/*sql statement:
		this command means:
			INSERT INTO sensors -> put data inside the "sensors" table
			(name) -> we are inserting into the "name" column
			VALUES (?) -> ? is a placeholder which will be replaced by the real data later
	*/
    	const char* sql = "INSERT INTO sensors (name) VALUES (?);";

	/*prepare the sql statement:
		sqlite3_prepare_v2 takes our SQL text and compiles it into something SQLite can execute, in other words convert
		english words into machine instructions.
	*/
    	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
		//if something goes wrong, print the error message
        	std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(m_db) << std::endl;
        	return false;
    	}

	/*bind the real value to the ?
		parameter indexes start at 1 and not 0
		SQLITE_TRANSIENT means SQLite will make its own copy of the string, so we don't have to worry about memory
	*/
	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

	/*execute the command:
		sqlite3_step runs the prepared statement, if it returns SQLITE_DONE means the insert worked
	*/
	if (sqlite3_step(stmt) != SQLITE_DONE){
        	std::cerr << "Insert failed: " << sqlite3_errmsg(m_db) << std::endl;
	     	//clean up memory
		sqlite3_finalize(stmt);
		return false;
	}

	/*clean up resources:
		sqlite3_finalize frees memory used by the statement
	*/
    	sqlite3_finalize(stmt); // clean up statement

	return true; //success
}


void SensorDatabase::loadSensors(std::vector<std::string>& names){
	/*reads data from the db, specifically:
		it looks inside the sensors table, it reads the name column and put all sensor names into a vector.
	*/

	//do nothing if db not opened
	if(!m_db)
		return;

	//clear the output vector so we don't mix old data with new data from the db
	names.clear();

	//write the SQL SELECT command that means, go to sensors table and give me (read) the values in the name column
	const char* sql = "SELECT name FROM sensors;";

	//prepare the sql statement, just like insert we need to prepare the stmt before running it
    	sqlite3_stmt* stmt = nullptr;

	//sqlit3_prepare_v2 converts the SQL text into executable form and it creates an sqlite3_stmt object(stmt)
	if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        	std::cerr << "Failed to prepare select statement: "<< sqlite3_errmsg(m_db) << std::endl;
        	return;
    	}

    	//loop through each row and create a Sensor object
    	while (sqlite3_step(stmt) == SQLITE_ROW){
        	/* sqlite3_column_text(stmt, 0) is a command that returns a const unsigned char* so we need to convert it into std::string
			stmt = current result row
			0 = first column (name)
		*/
        	std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        	//add the name into our vector
        	names.push_back(name);
    	}

	//clean up memory
    	sqlite3_finalize(stmt);
}


