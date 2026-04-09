#include "CalibrationRestorer.h"
#include "sensor/SensorManager.h"
#include "sensor/Interpolator.h"
#include <iostream>

/**
  * @brief constructs a CalibrationRestorer
 */
CalibrationRestorer::CalibrationRestorer(DatabaseManager* db, SensorManager* sensorManager, int projectId)
    		    : m_db(db), m_sensorManager(sensorManager), m_projectId(projectId)
{
}


/**
 * @brief restores calibration for every sensor in the provided vector
 */
void CalibrationRestorer::restore(const std::vector<std::unique_ptr<Sensor>>& sensors)
{
    	if(!m_db || !m_sensorManager)
        	return;

    	for(size_t i = 0; i < sensors.size(); ++i){
        	int sensorId = m_db -> getSensorID(sensors[i] -> getName());
        	int pin = sensors[i] -> getPin();

        	std::string type;
        	std::vector<CalibrationPoint> points;

        	//priority 1: if the sensor has a project-specific calibration (project + sensor + pin)
        	bool found = m_db -> loadProjectCalibration(m_projectId, sensorId, pin, type, points);

        	if(found){
            		std::cout << "Using project calibration for '" << sensors[i] -> getName() << "' pin=" << pin << "\n";
        	}
        	else{
            		//priority 2: if no, we check the global sensor template calibration
            		found = m_db -> loadGlobalCalibration(sensorId, type, points);

            		if(found)
                		std::cout << "Using global calibration for '" << sensors[i] -> getName() << "' pin=" << pin << "\n";
         	}

        	if(!found || points.empty()){
            		//priority 3: if neither, that means no calibration found, which means the sensor runs uncalibrated
            		std::cout << "No calibration found for '" << sensors[i] -> getName() << "' pin=" << pin << "\n";
            		continue;
        	}

        	//then we construct the interpolator and assign it to the sensor. Currently only 'table' type is implemented
		//when equation and datasheet calibration are added, we will use 'type' here
        	auto table = std::make_unique<std::vector<CalibrationPoint>>(points);
        	auto calibrator = std::make_unique<Interpolator>(std::move(table));
        	m_sensorManager -> setCalibration(i, std::move(calibrator));

        	std::cout << "Calibration restored for '" << sensors[i] -> getName() << "' pin=" << pin << "\n";
    	}
}
