#pragma once 
#include <vector>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

/**
 * @brief Stores a single run of data for one sensor, and exposes an API to add, clear, and retrieve stored data.
 *
 * DataSession is a simple container that stores the collected data for a single sensor.
 * Each sensor has its own DataSession object to hold all the values that we read from it.
 * This allows a direct associate of data to each Sensor.
 *
 * @note There is no class ownership relationship between Sensor and DataSession.
 * A DataSession is associated in name only with a specific sensor.
*/
class DataSession{
        
        public:
                //constructor to assign the sensorName    
                explicit DataSession(const std::string& sensorName);
		
		// Add a numeric reading from the sensor to the session
		void addValue(double value);
		
		//return all values collected so far & sensorName
		std::vector<double> getValues() const;
		std::string getSensorName() const;
		std::vector<double> getTimestamps() const;
		
		//clear all values to reset session
		void clear();
	
	private:
		//Name of the sensor, used in display and reference
		std::string m_sensorName;
		
		//STILL NOT SURE ABOUT THIS PART
		//mutable allows us to modify even data members that belong to a const object.
		//It's used here to protect m_value across threads.
		mutable std::mutex m_mutex;

		//List of the values collected
		std::vector<double> m_values;

		std::vector<double> m_timestamps;
		std::chrono::steady_clock::time_point m_startTime;
};

