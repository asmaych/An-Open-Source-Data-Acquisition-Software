#include "data/Run.h"

Run::Run(size_t runNumber)
    : m_runNumber(runNumber)
{
}

void Run::addFrame(double time, const std::vector<double>& values)
{
	//store time and the corresponding sensor values together
	m_times.push_back(time);
	m_frames.push_back(values);
}

//getters
size_t Run::getRunNumber() const
{
	return m_runNumber;
}

const std::vector<double>& Run::getTimes() const
{
	return m_times;
}

const std::vector<std::vector<double>>& Run::getFrames() const
{
	return m_frames;
}
