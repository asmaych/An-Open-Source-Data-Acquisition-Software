#include "data/DataSession.h"

//Constructor that initializes sensor name and keeps the vector of values empty
DataSession::DataSession(const std::string& sensorName)
	: m_sensorName(sensorName) 
{
}

//Add a value to the session by appending it to the vector of values
void DataSession::addValue(double Value)
{
	m_values.push_back(Value);
}

//Return all collected values by returning a copy of the values vector
std::vector<double> DataSession::getValues() const
{
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
	m_values.clear();
}
