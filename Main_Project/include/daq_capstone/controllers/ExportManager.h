#pragma once
#include <string>
#include <vector>
#include <memory>

class DataSession;

/* Export Manager is a class that writes sessions to either a csv or a simple text file
   It uses blocking I/O.
   the Caller/user decides the file path and permissions
*/

class ExportManager {

	public:
		//Export a single session to csv
		static bool exportSessionToCSV(const std::string& filepath, DataSession* session);

		//Export multiple sessions side-by-side
		static bool exportMultipleSessionsCSV(const std::string& filepath,
						      const std::vector<std::unique_ptr<DataSession>>& sessions);
};
