#include "controllers/ExportManager.h"
#include "data/DataSession.h"
#include <fstream>

bool ExportManager::exportSessionToCSV(const std::string& filepath, DataSession* session)
{
	//if no session, then no export
	if(!session){
		return false;
	}
	
	//Open output file stream for writing the csv
	std::ofstream out(filepath);

	//if the file couldn't open due to wrong path, or no permissions, then fail
	if(!out.is_open()){
		return false;
	}

	//We write the sensor name as the CSV header
	out << session -> getSensorName() << "\n";

	//write each value in the seesion on its own line
	for (double value : session -> getValues()){
		out << value << "\n";
	}

	//close the file
	out.close();
	return true; //Export succeeded
}

bool ExportManager::exportMultipleSessionsCSV(const std::string& filepath, const std::vector<std::unique_ptr<DataSession>>& session)
{
	//if there are no sessions, then false (no export)
	if(session.empty()){
		return false;
	}

	//Find the maximum number of rows (largest number of values among all sessions)
	size_t maxRows = 0;
	for(const auto& s : session){
		if(s){
			maxRows = std::max(maxRows, s -> getValues().size());
		}
	}

	//open the csv file for writing
	std::ofstream out(filepath);
	if(!out.is_open()){
		return false;
	}

	//for the header of the file we will use the sensor names (one sensor per column)
	for(size_t i = 0; i < session.size(); i++){
		//add comma between columns
		if(i){
			out << ",";
		}

		//if we don't have a name we use "sensor" by default
		out << (session[i]? session[i] -> getSensorName() : std::string("sensor"));
	}

	out << "\n";

	//write data rows where each row corresponds to index r in each session's values.
	for(size_t r = 0; r < maxRows; r++){
		for(size_t c = 0; c < session.size(); c++){
			if(c){
				//seperate columns with commas
				out << ",";
			}
			const auto& s = session[c];

			//if session exists and has a value in this row, write it
			if(s && r < s -> getValues().size()){
				out << s -> getValues()[r];
			}else{
				//empty cell for missing data 
				out << "";
			}
		}
		out << "\n";
	}

	out.close();
	return true; //export succeeded;
}
