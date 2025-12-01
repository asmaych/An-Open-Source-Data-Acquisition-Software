#pragma once
#include <wx/wx.h>
#include <wx/grid.h> // Grid widget used for displaying and editing tabular data
#include <memory>
#include "data/DataSession.h"

/* DataTableWindow is a single table window that displays collected sensor values in a table
   After an experiment, we can show all values collected for a sensor in a structured table (wxGrid) where each value appears
   in a seperate row
   It supports dynamic update (append/remove columns as needed)
*/

class DataTableWindow : public wxFrame
{
	public:
		DataTableWindow(wxWindow* parent, const std::vector<std::shared_ptr<DataSession>>& sessions);

		//update table with latest values
		void updateTable();

		//set which sensors to display (dynamic)
		void setSelectedSessions(const std::vector<std::shared_ptr<DataSession>>& sessions);

		//append a single value to a specific column
		void appendRow(const std::vector<double>& rowValues);
	private:
		std::vector<std::shared_ptr<DataSession>> m_sessions; //sessions currently displayed
		wxGrid* m_grid; //Grid widget to display the table

};
