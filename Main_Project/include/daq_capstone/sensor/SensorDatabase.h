#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include "Sensor.h"

class Sensor;

/* SensorDatabase is the class responsible for saving and loading sensors to and from an SQLite database file.
   We are using the database to store the sensors, cause without it they disappear as soon as the app closes.
   This class creates a database file if it doesn't exist, and a sensors table that will be saved in the db and loaded
   back into memory.
*/

class SensorDatabase
{
	public:
		//constructor & destructor
		SensorDatabase();
		~SensorDatabase();

		// ===== database control =====
		//opens or creates the database file ex: "sensors.db"
		bool open(const std::string& path);

		//closes the database safely
		void close();

		// ====== save/load sensors ======
		//saves all sensors to the database, existing sensor rows will be deleted first
		void saveSensors(const std::vector<std::unique_ptr<Sensor>>& sensors);

		//loads sensors from the db into the given vector, the vector is cleared before loading
		void loadSensors(std::vector<std::unique_ptr<Sensor>>& sensors);

	private:
		//creates database tables if they do not exist
		void createTables();

		//SQLite db handle/object
		sqlite3* m_db = nullptr;
};
