#include "Sensor.h"

//Constructor
Sensor::Sensor(const std::string& name, int id)
	: name (name), id (id), reading(0)
{
}

// get/return sensor name
std::string Sensor::getName() const
{
	return name;
}

// get sensor ID
int Sensor::getID() const
{
	return id;
}

// get current reading
int Sensor::getReading() const
{
	return reading.load(); 
	/* load is a thread safe read operation that retrieves a value from shared memory without interruption. It guarantess
	   that the entire value is read as a single, complete unit, preventing other threads from seeing a corrupted value
	   during read process.
	*/
}

// set new reading
void Sensor::setReading(int value)
{
	reading.store(value);  
	/* store is an atomic operation that writes a value to a memory location as a single, indivisible action, ensuring that
	   it's either fully completed or not at all. (i am using atomic (thread=safe) again to guarantee that other processes
	   will never see a partially updated value. 
	*/
}
