#include "data/DataTableWindow.h"

//constructor to set up the window and grid
DataTableWindow::DataTableWindow(wxWindow* parent, DataSession* session)
	: wxFrame(parent, wxID_ANY, "Collected Data", wxDefaultPosition, wxSize(500,400)), m_session(session)
{
	//create a wxGrid for table display
	m_grid = new wxGrid(this, wxID_ANY);
	m_grid -> CreateGrid(0,1); //0 rows and 1 column initially
	m_grid -> SetColLabelValue(0, "Values"); //set header name
	
	populateGrid(); //fill table with current session values
}

//PopulateGrid() will fill the grid with the collected data from session
void DataTableWindow::populateGrid()
{
	const std::vector<double>& values = m_session -> getValues();
	m_grid -> ClearGrid(); //Clear any existing values from the grid(remove old data)
	
	//Ensure the grid has enough rows to display all values.
	//If the current number of rows is less than the number of data points, we append more
	if (m_grid -> GetNumberRows() < (int)values.size()){
		m_grid -> AppendRows(values.size() - m_grid -> GetNumberRows());
	}
	
	//loop through values and fill cells
	for (size_t i = 0; i < values.size(); ++i) //size_t is a safe type for indexing vectors (always non)negative
	{
		//we convert the double to a string with 2 decimal places and we set the value of the grid at row i, column 0
		// i am not updating column cause for now i am keeping it simple one column one set of values
		m_grid -> SetCellValue(i, 0, wxString::Format("%.2f", values[i]));
	}
}
