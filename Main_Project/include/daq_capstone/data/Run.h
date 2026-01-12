#pragma once
#include <vector>

/* Run represents One Continuous acquisition session.
	- Each run has many time stamped frames where each frame contains values from all sensors.
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
