#include <vector>
#include <memory>
#include "data/DataSession.h"

/*
	DataCollector is a lightweight helper for the 2 diff methods of collecting that we have:
	1. continuous collect which append incoming values into their sessions
	2. collect last-value which append the last value received from sensors
	Thus, this class provides two methods that the GUI can call. it does not own sessions.
*/

enum class CollectionMode {Continuous, LastValue};

class DataCollector
{
	public:
		DataCollector() = default;

		void setMode(CollectionMode m) { m_mode = m; }
		CollectionMode getMode() const { return m_mode; }

		//Append a single value from polling into the correct session index
		static void collectContinuous(std::vector<std::unique_ptr<DataSession>>& sessions, size_t index, double value);

		//Snapshot collect, appending last value for each selected sensor
		static void collectLastValue(std::vector<std::unique_ptr<DataSession>>& sessions,
					     const std::vector<size_t>& indices,
					     const std::vector<int>& lastValues);

		// a helper to collect last values for ALL sensors (indices = 0..n-1)
    		void collectLastValueAll(std::vector<std::unique_ptr<DataSession>>& sessions,
                                         const std::vector<int>& latestValues);

	private:
		CollectionMode m_mode = CollectionMode::Continuous;
};

