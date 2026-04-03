#pragma once
#include <vector>
#include <memory>
#include "sensor/Sensor.h"
#include "db/DatabaseManager.h"
#include "CalibrationPoint.h"

class SensorManager;

/**
  * @brief restores saved calibrations from the database onto in-memory sensors, called once during loadProjectFromDatabase() after 
  * sensors and runs are fully loaded.
*/
class CalibrationRestorer
{
    	public:
        	/**
         	  * @brief constructs a CalibrationRestorer
         	  *
         	  * @param db             raw pointer to the shared DatabaseManager
         	  * @param sensorManager  raw pointer to the project's SensorManager
         	  * @param projectId      DB ID of the project being loaded
         	*/
        	CalibrationRestorer(DatabaseManager* db, SensorManager* sensorManager, int projectId);

        	/**
         	  * @brief restores calibration for every sensor in the provided vector
         	  *
         	  * For each sensor, the priority is:
         	  *   1. project_calibrations (project + sensor + pin) wins if found
         	  *   2. sensor_calibrations (global template) fallback
         	  *   3. Uncalibrated sensor runs raw
         	  *
         	  * An Interpolator is constructed from the loaded points and assigned to the sensor through 
		  * SensorManager::setCalibration() so readings are immediately calibrated without requiring a project reload
         	  *
         	  * @param sensors  The project's in-memory sensor vector
         	*/
        	void restore(const std::vector<std::unique_ptr<Sensor>>& sensors);

    	private:
        	DatabaseManager* m_db = nullptr;

		SensorManager* m_sensorManager = nullptr;

		int m_projectId = -1;
};
