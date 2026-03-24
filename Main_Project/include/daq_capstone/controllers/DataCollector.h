#include <vector>
#include <memory>
#include "data/DataSession.h"

enum class CollectionMode {Continuous, LastValue};

/**
 * @brief A lightweight helper class for the different methods of recording sensor values
 *
 * DataCollector is a lightweight helper for the 2 different methods of collecting that we have:
 *	1. continuous-collect that continuously appends incoming sensor readings into the current session until stopped
 *	2. collect-last-value that appends only the last set of values received once per invocation
 *
 * This class provides two methods that the GUI can call. it does not own sessions.
 */
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

		//A helper to collect last values for ALL sensors (indices = 0..n-1)
    		void collectLastValueAll(std::vector<std::unique_ptr<DataSession>>& sessions,
                                         const std::vector<int>& latestValues);

	private:
		CollectionMode m_mode = CollectionMode::Continuous;
};

