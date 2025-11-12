#pragma once
#include <wx/wx.h>
#include <wx/grid.h> // Grid widget used for displaying and editing tabular data
#include "DataSession.h"

/* DataTableWindow is a class that displays the collected data in a table
   After an experiment, we can show all values collected for a sensor in a structured table (wxGrid) where each value appears
   in a seperate row
*/

class DataTableWindow : public wxFrame
{
	public:
		//constructor with parent window (MainFrame) and a pointer to Datasession to display collected data
		DataTableWindow(wxWindow* parent, DataSession* session);

	private:
		DataSession* m_session; //the collected data that we will display
		wxGrid* m_grid; //Grid widget to display the table

		//helper function to populate the grid from the session (fill each row and column with the values from session)
		void populateGrid();
};
