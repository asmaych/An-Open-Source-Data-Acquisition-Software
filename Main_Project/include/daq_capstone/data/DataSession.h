#pragma once 
#include <vector>
#include <string>
#include <fstream>

/* DataSession is a simple container that stores the collected data for a single sensor. 
   Each sensor has its own DataSession object to hold all the values that we read from the sensor which keeps data organized per     
   sensor. 
*/

class DataSession{
        
        public:
                //constructor to assign the sensorName    
                DataSession(const std::string& sensorName);
		
		// Add a numeric reading from the sensor to the session
		void addValue(double value);
		
		//return all values collected so far & sensorName
		std::vector<double> getValues() const;
		std::string getSensorName() const;
		
		//clear all values to reset session
		void clear();
	
	private:
		std::string m_sensorName; //Name of the sensor, used in displau and reference
		std::vector<double> m_values; //List of the values collected
};
