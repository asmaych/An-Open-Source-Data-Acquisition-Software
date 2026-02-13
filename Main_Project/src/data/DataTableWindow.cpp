#include "data/DataTableWindow.h"
#include "data/Run.h"

//constructor to set up the window and grid
DataTableWindow::DataTableWindow(wxWindow* parent, const std::vector<std::shared_ptr<DataSession>>& sessions,
				 std::shared_ptr<Run> run)
	: wxPanel(parent),
          m_sessions(sessions),
	  m_associatedRun(run)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	//create a wxGrid for table display
	m_grid = new wxGrid(this, wxID_ANY);
	m_grid -> CreateGrid(0, sessions.size()); //O row and size column initially
	//m_grid -> DeleteRows(0);

	//set column headers to the sensor names
	for(size_t i = 0; i < sessions.size(); ++i){
		m_grid -> SetColLabelValue(i, sessions[i] -> getSensorName());
	}

	//disable editing of cells by the user (read-only)
	m_grid -> EnableEditing(false);

        //automatically size columns to fit the contents
        m_grid -> AutoSizeColumns();

	sizer -> Add(m_grid, 1, wxEXPAND | wxALL, 5);

	m_collect_button = new wxButton(this, wxID_ANY, "Collect");
	m_collect_button -> Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
		if (!m_associatedRun)
        		return;

    		const auto& frames = m_associatedRun->getFrames();
    		const auto& times  = m_associatedRun->getTimes();

    		if (frames.empty() || times.empty())
        		return;

    		size_t lastIndex = times.size() - 1;

    		double timestamp = times[lastIndex];
    		const std::vector<double>& sensorValues = frames[lastIndex];

    		std::vector<double> row;
    		row.push_back(timestamp);
    		row.insert(row.end(), sensorValues.begin(), sensorValues.end());

    		appendRow(row);

	});

	sizer -> Add(m_collect_button, 0, wxALIGN_RIGHT | wxALL, 5);

	SetSizer(sizer);
}


//set which Datasessions (sensors) should be displayed in the grid
void DataTableWindow::setSelectedSessions(const std::vector<std::shared_ptr<DataSession>>& sessions)
{
	m_sessions = sessions;

	//update the number of columns to match the number of selected sensors
	int colCount = static_cast<int>(sessions.size());

	//clear all existing cell values
	m_grid -> ClearGrid();

	//adjust columns: append if we have fewer columns than needed
	if(m_grid -> GetNumberCols() < colCount){
		m_grid -> AppendCols(colCount - m_grid -> GetNumberCols());
	}

	//delete extra columns if we have more than needed
	else if(m_grid -> GetNumberCols() > colCount){
		m_grid -> DeleteCols(colCount, m_grid -> GetNumberCols() - colCount);
	}

	//update column headers to match the new sensor selection
	for(size_t i = 0; i < m_sessions.size(); ++i){
		m_grid -> SetColLabelValue(i, m_sessions[i] -> getSensorName());
	}

	//force the grid to redraw itself
	m_grid -> ForceRefresh();
}


//update the grid with the latest sensor data
void DataTableWindow::updateTable()
{

	//if no sessions are selected, do nothing
	if(m_sessions.empty())
		return;

	//determine the max number of rows required to fit all sensor data
	size_t maxRows = 0;

	for(auto& session : m_sessions){
		maxRows = std::max(maxRows, session -> getValues().size());
	}

	int currentRows = m_grid -> GetNumberRows();

	//add more rows if needed
	if(currentRows < static_cast<int>(maxRows)){
		m_grid -> AppendRows(static_cast<int>(maxRows) - currentRows);
	}

	//fill out the grid with sensor data
	for(size_t col = 0; col < m_sessions.size(); ++col){

		//get sensor readings
		auto values = m_sessions[col] -> getValues();

		for(size_t row = 0; row < values.size(); ++row){

			//set cell value with 2 decimal places
			m_grid -> SetCellValue(static_cast<int>(row), static_cast<int>(col), wxString::Format("%.2f", values[row]));
		}
	}

	//resize columns to fit new content
	m_grid -> AutoSizeColumns();

	//refresh the grid to ensure all updates are shown
	m_grid -> ForceRefresh();
}

void DataTableWindow::appendRow(const std::vector<double>& rowValues)
{
	if(rowValues.empty())
		return;

	// ============== STORE THE DATASHEET ===============
	double t = rowValues[0]; //time column
	m_times.push_back(t);

	size_t sensorCount = rowValues.size() - 1;

	//first time: create storage for each sensor
	if(m_values.empty())
		m_values.resize(sensorCount);

	if(m_values.size() != sensorCount){
		m_values.clear();
		m_values.resize(sensorCount);
	}

	for(size_t i = 0; i < sensorCount; ++i)
		m_values[i].push_back(rowValues[i + 1]);

	// ================= UPDATE THE GRID ================
	//make sure grid has enough columns
	if(m_grid -> GetNumberCols() < (int) rowValues.size())
		m_grid -> AppendCols(rowValues.size() - m_grid -> GetNumberCols());

	//append a new row
	int newRow = m_grid->GetNumberRows();
    	m_grid->AppendRows(1);

	//fill the row
    	for(size_t col = 0; col < rowValues.size(); ++col)
    	{
		if(std::isnan(rowValues[col])){
        		m_grid->SetCellValue(newRow, col, "0.0");
    		}else{
			if(col == 0){
				//timestamp (double seconds)
				wxString tsStr = wxString::Format("%.3f", rowValues[col]);
				m_grid -> SetCellValue(newRow, col, tsStr);
			} else{
				m_grid -> SetCellValue(newRow, col, wxString::Format("%.2f", rowValues[col]));
			}
		}
	}

    	m_grid -> AutoSizeColumns();
	m_grid -> MakeCellVisible(newRow, 0);
    	m_grid -> ForceRefresh();
}

std::shared_ptr<Run> DataTableWindow::getAssociatedRun() const
{
	return m_associatedRun;
}

//apply the selected theme (dark or light) to the data table window
void DataTableWindow::applyTheme(Theme theme)
{
	//determine background and foreground colors based on theme
	wxColour bg, fg, gridLine;

	if(theme == Theme::Dark){
		bg = wxColour(30, 30, 30);
		fg = wxColour(220, 220, 220);
	} else if (theme == Theme::Light){
    		bg = *wxWHITE;
   		fg = *wxBLACK;
	}
	else {
		bg = GetParent() -> GetBackgroundColour();
		fg = GetParent() -> GetForegroundColour();
	}

	//apply the colors to the wxGrid widget that displays the tabe
	SetBackgroundColour(bg); // grid background
	SetForegroundColour(fg); //text color
	if(m_grid){
		m_grid -> SetBackgroundColour(bg); //sets the overall grid background
		m_grid -> SetForegroundColour(fg); //sets the text color for all cells
		m_grid -> SetDefaultCellBackgroundColour(bg);
    		m_grid -> SetDefaultCellTextColour(fg);
   		m_grid -> SetLabelBackgroundColour(bg);
    		m_grid -> SetLabelTextColour(fg);
		m_grid -> ClearSelection();
		m_grid -> ForceRefresh();
	}

	if(m_collect_button) {
	    	m_collect_button -> SetBackgroundColour(bg);
		m_collect_button -> SetForegroundColour(fg);
	}

	Layout();
	Refresh();
}
