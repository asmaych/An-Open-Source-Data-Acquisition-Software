#pragma once

#include <vector>
#include <string>
#include "Sensor.h"
#include "SerialComm.h"
#include <memory>
#include <functional>
#include "Calibrator.h"
#include "Interpolator.h"

class SensorManager
{
	public:
		//constructor that takes an address to a vector of Sensors
		//and a raw pointer to the serialComm object owned by Project
		SensorManager(std::vector<std::unique_ptr<Sensor>>& sensors, SerialComm* serialComm);

		//a general method for adding new sensors
		bool addSensor(std::unique_ptr<Sensor> s);

		//a general method for removing existing sensors
		bool removeSensor(const std::string& sensorName);

		//checker function to make sure we don't have duplicate names
		bool nameExists(const std::string& name);

		//checker function to make sure we don't have two sensors using
		//the same arduino pin number
		bool pinExists(int pin) const;

		void setAllReadingStrategy(const std::string &reading_strategy);

		void setReadingStrategy(long sensor_index, const std::string &reading_strategy);

		void setCalibration(long sensor_index, std::unique_ptr<Calibrator> calibrator);

		std::vector<CalibrationPoint> const *getSensorCalibration(long sensor_index) const;

		//helper to get all selected sensors
		std::vector<Sensor*> getSelectedSensors() const;
		void setOnChangeCallback(std::function<void()> cb);

	private:
		//-------------------------------------------------------------------------------------------
		//CLASS MEMBERS
		//-------------------------------------------------------------------------------------------

		//reference to a vector of Sensors. This will be assigned to
		//the vector owned by Project, so that this class can modify it
		std::vector<std::unique_ptr<Sensor>>& m_sensors;

		//raw pointer to the SerialComm object owned by Project
		SerialComm* m_serialComm;

		std::function<void()> m_onChange;
};
