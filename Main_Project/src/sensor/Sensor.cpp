#include "sensor/Sensor.h"

//Constructor
Sensor::Sensor(const std::string& name, int pin)
{
	/* \brief This class takes a name and an integer "pin" 
	 * as parameters.
	 *
	 * the pin represents the arduino/esp32 pin that will
	 * be used to read sensor values.
	 *
	 * the name represents what the sensor will be called
	 * and displayed to the user as.
	 *
	 * Both name and pin must be unique, but that will be
	 * enforced in an owning class SensorManager object.
	 */
	m_name = name;
	m_pin = pin;
	m_reading = 0;	//initialize to zero on creation
}

// get/return sensor name
std::string Sensor::getName() const
{
	return m_name;
}

// get sensor ID
int Sensor::getPin() const
{
	return m_pin;
}

// get current reading
int Sensor::getReading() const
{
	return m_reading.load(); 
	/* load is a thread safe read operation that retrieves 
	 * a value from shared memory without interruption. 
	 * It guarantees that the entire value is read as a 
	 * single, complete unit, preventing other threads from 
	 * seeing a corrupted value during read process.
	*/
}

// set new reading
void Sensor::setReading(int value)
{
	m_reading.store(value);  
	/* store is an atomic operation that writes a value to a 
	 * memory location as a single, indivisible action, ensuring 
	 * that it's either fully completed or not at all. (i am using 
	 * atomic (thread-safe) again to guarantee that other processes
	 * will never see a partially updated value. 
	*/
}
