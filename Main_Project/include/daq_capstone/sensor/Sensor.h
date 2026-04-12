#pragma once

#include <string>
#include <atomic> 
#include <iosfwd>
#include <mutex>
#include <memory>
#include <vector>

#include "CalibrationPoint.h"
#include "Calibrator.h"
#include "ReadingStrategy.h"
#include "MappedReader.h"

/* #include <atomic>: for thread-safe reading, it ensures that 
 * operations on a variable are performed atomically 
 * (uninterruptible and safe for multithreaded environments).
 *
 * #include <mutex>: provides us with a sychronization primitive 
 * that is used to protect the shared data from being accessed 
 * by multiple threads simultaneously.
 */

/**
 * @brief Owned by Sensor, and used to store a packet of Raw, Voltage, and mapped readings.
 *
 * This struct is critical in the ReadingStrategy Design Pattern. It is passed by reference to the ReadingStrategy
 * implementation, allowing one of the three values to be returned.
 */
struct ReadingPacket {
	std::atomic<double> Raw;
	std::atomic<double> Voltage;
	std::atomic<double> Mapped;
};


/**
 * @brief Virtual sensor used to reflect the real-world state of a single physical sensor
 *
 * For each sensor connected to the microcontroller, there will be one instance of Sensor. Each sensor has its own
 * set of readings, and exposes a public API that allows readings to be set, and retrieved from the Sensor.
 *
 * Additionally, each Sensor uses its own Calibrator object, which modifies the behavior of the getReading() method
 * according to user-defined calibrations. If no calibration is specified, raw values are returned. Having each
 * sensor own its own calibration allows for per-sensor calibration.
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

	/* This is a function that uses the owned ReadingStrategy derived
	 * class to return the value from the struct ReadingPacket that
	 * corresponds to the derived class:
	 *	- VoltageReader		-> returns ReadingPacket.Voltage
	 *	- RawReader			-> returns ReadingPacket.Raw
	 *	- MappedReader		-> returns ReadingPacket.Mapped
	 *
	 * The reason for implementing the getter in this way is to allow
	 * for runtime polymorphism, where the user can choose which of the
	 * three values to collect during runtime.
	 */
	std::atomic<double> getReading() const;

	void setReadingStrategy(std::unique_ptr<ReadingStrategy> readingStrategy);

	/* this is a function that is used to assign a Calibrator object
	 * to this sensor instance. The calibrator will be configured by
	 * the user in a GUI, SensorManager will handle the backend config,
	 * and then it will send it to this sensor using this method
	 */
	void setCalibrator(std::unique_ptr<Calibrator> calibrator);

	std::vector<CalibrationPoint> const *getCalibration() const;


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
	bool m_selected = true;
	std::unique_ptr<Calibrator> m_calibrator;	//pointer to a calibrator object
	ReadingPacket m_readingPacket;	//struct containing thread safe raw, voltage, and mapped values

	//set the default behavior for the reading strategy to the derived class MappedReader
	std::unique_ptr<ReadingStrategy> m_readingStrategy = std::make_unique<MappedReader>();

};
