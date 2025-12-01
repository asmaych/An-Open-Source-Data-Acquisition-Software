#include "controllers/DataCollector.h"
#include "data/DataSession.h"
#include <algorithm> //for std::min

/* This function collects a contiuous data value into a specific session
	sessions: a vector of unique pointers to datasessions objects
	index: which session in the vector to add the value to
	value: THe data value to add (double)
*/

void DataCollector::collectContinuous(std::vector<std::unique_ptr<DataSession>>& sessions, size_t index, double value)
{
	//we check if the index is valid and the pointer at session[index] is not null, if either conditions faills we do nothing
	if(index >= sessions.size() || !sessions[index]){
		return;
	}
	sessions[index] -> addValue(value);
}

void DataCollector::collectLastValue(std::vector<std::unique_ptr<DataSession>>& sessions,
				     const std::vector<size_t>& indices,
				     const std::vector<int>& latestValues)
{
	/*LatestValues must be same length as indices
	  we compute the number of iteratios n as the smaller of the two vectors which prevents out of bound access if the two
	  vectors have diff lengths
	*/
	size_t n = std::min(indices.size(), latestValues.size());
	
	// now we loop over each pair of (index, value)
	// index is the session index from indices
	for(size_t i = 0; i < n; ++i){
		size_t index = indices[i];
		
		//we check if the session index is valid, and its pointer isn't null
		if(index < sessions.size() && sessions[index]){
			//we add the corresponding latest value to the session, since latestValue[i] is int and addValue is double
			// we need to cast it to double
			sessions[index] -> addValue(static_cast<double>(latestValues[i]));
		}
	}
}

// Convenience: collect last values for all sessions (indices 0..N-1)
void DataCollector::collectLastValueAll(std::vector<std::unique_ptr<DataSession>>& sessions,
                                        const std::vector<int>& latestValues)
{
        size_t n = std::min(sessions.size(), latestValues.size());
    	for (size_t i = 0; i < n; ++i) {
        	if (sessions[i]){
			sessions[i]->addValue(static_cast<double>(latestValues[i]));
    		}
    	}
}
