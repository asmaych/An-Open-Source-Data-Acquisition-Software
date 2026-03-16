#pragma once

#include <string>
#include <atomic> 
#include <iosfwd>
#include <mutex>
#include <memory>
#include <bits/stl_vector.h>

#include "CalibrationPoint.h"
#include "Calibrator.h"

/* #include <atomic>: for thread-safe reading, it ensures that 
 * operations on a variable are performed atomically 
 * (uninterruptible and safe for multithreaded environments).
 *
 * #include <mutex>: provides us with a sychronization primitive 
 * that is used to protect the shared data from being accessed 
 * by multiple threads simultaneously.
 */

class Sensor 
{

public: 

	//Constructor: initializes a sensor with a name and a pin number
	Sensor(const std::string& name, int pin);
	
	//---------------------------------------------------------------
	//GETTERS
	//---------------------------------------------------------------
	
	std::string getName() const;
	int getPin() const;
	 
	/* a getter to  get the current reading (thread-safe meaning 
	 * it can be called from multiple threads (a sequence of 
	 * instructions that can run concurrently within a program)  
	 * at the same time without causing errors, crashes, or incorrect 
	 * results.
	 */
	double getMappedReading() const;

	/* this is a function that is used to assign a Calibrator object
	 * to this sensor instance. The calibrator will be configured by
	 * the user in a GUI, SensorManager will handle the backend config,
	 * and then it will send it to this sensor using this method
	 */
	void setCalibrator(std::unique_ptr<Calibrator> calibrator);

	std::vector<CalibrationPoint> const *getCalibration();

	/* a similar getter to getMappedReading, but this retrieves the 
	 * raw sensor value of 0-4096, before any calibration
	 */
	int getRawReading() const;

	/* one more getter whose job is to return the value of the voltage
	 * that corresponds to the value that has been mapped to 0 - 4096.
	 * It is essentially the inverse function to the analog-to-digital
	 * conversion that happens in the ESP32
	 */
	float getVoltage() const;

	
	//---------------------------------------------------------------
	//SETTERS
	//---------------------------------------------------------------

	// a setter for current reading (again thread-safe)
	void setReading(int raw_value);

	void setSelected(bool sel) { m_selected = sel; }
        bool isSelected() const { return m_selected; }


private:
	
	std::string m_name;
	int m_pin;
	std::atomic<double> m_mappedreading;  		//thread-safe mapped reading value
	std::atomic<int> m_rawreading;			//thread safe raw reading value
	std::atomic<float> m_voltage;			//thread safe voltage reading value
	bool m_selected = true;
	std::unique_ptr<Calibrator> m_calibrator;	//pointer to a calibrator object
};
