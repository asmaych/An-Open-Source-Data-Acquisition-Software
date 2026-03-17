#include <vector>
#include "SensorManager.h"
#include "Sensor.h"
#include <algorithm>
#include <string>
#include <memory>
#include <iostream>

#include "RawReader.h"
#include "VoltageReader.h"
#include "MappedReader.h"

SensorManager::SensorManager(std::vector<std::unique_ptr<Sensor>>& sensors, SerialComm* serialComm):
	m_sensors(sensors),
	m_serialComm(serialComm)
{
	/* \brief 	This is the constructor for the SensorManager class
	 *		It takes by reference a vector of Sensor objects, and
	 *		a raw pointer to the SerialComm object owned by Project,
	 *		and will use the reference to modify the vector, and 
	 *		the pointer to send the appropriate add and remove 
	 *		commands to the microcontroller
	 */
}

void SensorManager::setOnChangeCallback(std::function<void()> cb) {
    m_onChange = std::move(cb);
}


bool SensorManager::addSensor(std::unique_ptr<Sensor> s)
{
	/* \brief 	This function takes a reference to a Sensor object 
 	* 		and validates its configuration. If the Sensor is
 	* 		valid, then it is added and the function returns 
 	* 		true, otherwise nothing happens, and the function
 	* 		returns false.
 	*/

	std::cout << "oopsy looks like we've been called to add a new sensorrr\n";

	//if either the name or pin already exist:
	if (nameExists(s->getName()) || pinExists(s->getPin()))
	{
		//do not modify the vector, return false
		std::cout << "Pin already used!\n";
		return false;
	}

	std::cout << "looks like the name and pin for the new sensor are okay!\n";

	//we register it with the microcontroller:
	m_serialComm->addSensor(s->getName(), s->getPin());
	std::cout << "successfully registered the sensor with the arduino!\n";	

	//otherwise, we add the Sensor to the vector:
	m_sensors.push_back(std::move(s));
	std::cout << "successfully put the new sensor in the vector!\n";
	
	if(m_onChange) m_onChange();
	//and terminate the function with true
	return true;
}

bool SensorManager::removeSensor(const std::string& sensorName)
{
	/* \brief 	This function takes the name of a sensor in the 
	 * 		vector, and uses it to search for, and remove the
	 * 		corresponding Sensor from the vector of Sensors
	 * 		owned by Project, and referenced by m_sensors .
	 */

	//use an iterator to traverse the list. At the end, std::remove_if()
	//will move the selected object to to end of the vector, and point
	//the iterator to it.
	auto it = std::remove_if(
			m_sensors.begin(),
			m_sensors.end(),
			[&](const std::unique_ptr<Sensor>& s){return s->getName() == sensorName; });

	//if our iterator got to the end without finding the sensor
	//then there is some kind of problem with the naming, so we
	//return false, and do nothing to the vector of Sensors
	if (it == m_sensors.end())
	{
		return false;
	}

	//otherwise, we have found it somewhere, and the iterator
	//"it" points to it, so we just erase it, remove it from
	//the registry in the microcontroller, and return true
	m_sensors.erase(it, m_sensors.end());

	//remove the sensor from the microcontroller as well
	m_serialComm->removeSensor(sensorName);
	
	if(m_onChange) m_onChange();

	return true;
}

bool SensorManager::nameExists(const std::string& name)
{

	/* \brief	This function takes a name as a parameter, and returns
	 * 		true if the name is already in used, otherwise it
	 * 		returns false
	 */
	return std::any_of(
			m_sensors.begin(),
		       	m_sensors.end(),
			[&](const std::unique_ptr<Sensor>& s){return s->getName() == name; });
}

bool SensorManager::pinExists(int pin) const
{
	/* \brief	This function takes an int pin as a parameter, and
	 * 		returns true if no other sensor is already using the
	 * 		pin passed as a parameter, otherwise it returns false
	 */

	return std::any_of(
			m_sensors.begin(),
			m_sensors.end(),
			[&](const std::unique_ptr<Sensor>& s){return s->getPin() == pin; });
}

void SensorManager::setAllReadingStrategy(const std::string &reading_strategy) {
	for (size_t i = 0; i < m_sensors.size(); i++) {
		setReadingStrategy(i, reading_strategy);
	}
}

void SensorManager::setReadingStrategy(const long sensor_index, const std::string &reading_strategy) {
	if (reading_strategy == "Raw") {
		auto strategy = std::make_unique<RawReader>();
		m_sensors[sensor_index]->setReadingStrategy(std::move(strategy));
	}
	else if (reading_strategy == "Voltage") {
		auto strategy = std::make_unique<VoltageReader>();
		m_sensors[sensor_index]->setReadingStrategy(std::move(strategy));
	}
	else if (reading_strategy == "Mapped") {
		auto strategy = std::make_unique<MappedReader>();
		m_sensors[sensor_index]->setReadingStrategy(std::move(strategy));
	}
	}

void SensorManager::setCalibration(long sensor_index, std::unique_ptr<Calibrator> calibrator)
{
	m_sensors[sensor_index]->setCalibrator(std::move(calibrator));
}

std::vector<CalibrationPoint> const * SensorManager::getSensorCalibration(const long sensor_index) const {
	return m_sensors[sensor_index]->getCalibration();
}

std::vector<Sensor*> SensorManager::getSelectedSensors() const {
	std::vector<Sensor*> selected;
	for (auto& s : m_sensors) {
		if (s->isSelected()) {
			 selected.push_back(s.get());
        	}
    	}
    	return selected;
}
