#include "data/DataSession.h"

//Constructor that initializes sensor name and keeps the vector of values empty
DataSession::DataSession(const std::string& sensorName)
	: m_sensorName(sensorName) 
{
}

//Add a value to the session by appending it to the vector of values
void DataSession::addValue(double Value)
{
	/* std::lock_guard is a RAII=based mechanism in C++ that simplifies mutex management by automatically
	   locking a mutex upon creation and keeps reference to it and unlocking it upon destruction (when the scope ends).
	*/ 
	std::lock_guard<std::mutex> lock(m_mutex);
	m_values.push_back(Value);
}

//Return all collected values by returning a copy of the values vector
std::vector<double> DataSession::getValues() const
{
	// return a copy under lock to avoid races
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_values;
}

//REturn sensorName
std::string DataSession::getSensorName() const
{
	return m_sensorName;
}

//clear all stored values to reset session with a clean/empty values vector
void DataSession::clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_values.clear();
}
