#pragma once
#include <vector>

/**
 * @brief Holds all timestamped sensor readings from all sensors for one run of data collection.
 *
 * When the user presses the Toolbar Start button, A run of data will begin - as long as there is at least one sensor.
 * Sensor Values will be recorded and stored together in timestamped frames.
 *
 * When the user presses the Toolbar Stop button, the run will end, and subsequently pressing the Start button again
 * will result in a different, new run.
*/
class Run
{

public:
	explicit Run(size_t runNumber);

	//add one full frame (time + all sensor values)
	void addFrame(double time, const std::vector<double>& values);

	//getters
	size_t getRunNumber() const;
	const std::vector<double>& getTimes() const;
	const std::vector<std::vector<double>>& getFrames() const;

private:
	//Run 1, Run 2, ...
	size_t m_runNumber;
	//t0, t1, t2 ...
	std::vector<double> m_times;
	//[ [s1, s2, s3 ...], ...]
	std::vector<std::vector<double>> m_frames;
};
