#include "data/LiveDataWindow.h"
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/app.h>

//Constructor: sets up the GUI window
LiveDataWindow::LiveDataWindow(wxWindow* parent)
		: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	//notebook allows multiple runs as tabs
	m_notebook = new wxNotebook(this, wxID_ANY);
	//m_notebook -> SetMinSize(wxSize(300,250));

	sizer -> Add(m_notebook, 1, wxEXPAND | wxALL, 5);
	SetSizer(sizer);

	Layout();
}

//creates a new tab and marks it as active
void LiveDataWindow::startNewRun(std::shared_ptr<Run> run, const std::vector<std::unique_ptr<Sensor>>& sensors)
{
	m_activeRun = run;

	m_sensorNames.clear();
	for(auto& sensor : sensors){
		m_sensorNames.push_back(sensor -> getName());
	}

	//create grid
	wxGrid* grid = new wxGrid(m_notebook, wxID_ANY);
	grid -> SetMinSize(wxSize(200,150));
	grid -> CreateGrid(0, static_cast<int>(m_sensorNames.size() + 1)); // +1 for time
	grid -> EnableEditing(false); //read-only
	grid -> SetRowLabelSize(40);

	//required for GTK stability
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	grid -> SetDefaultCellFont(font);
	grid -> SetLabelFont(font);

	//column headers
	grid -> SetColLabelValue(0, "Time (s)");

	for(size_t i = 0; i < m_sensorNames.size(); ++i){
		grid -> SetColLabelValue(i + 1, m_sensorNames[i]);
	}

	//grid -> AutoSizeColumns();

	wxString title = wxString::Format("Run %zu", run -> getRunNumber());

	m_notebook -> AddPage(grid, title, true);

	m_activeGrid = grid;

	//autosize only after GTK realizes the tab
	CallAfter([this](){
		if(m_activeGrid)
			m_activeGrid -> AutoSizeColumns();
	});

	Layout();
}

//prevents further writing to the run
void LiveDataWindow::stopRun()
{
	m_activeRun.reset();
	m_activeGrid = nullptr;
}

//append one frame to the currently active run
void LiveDataWindow::addFrame(double time, const std::vector<double>& values)
{
	if(!m_activeGrid)
		return;


	int newRow = m_activeGrid -> GetNumberRows();
	m_activeGrid -> AppendRows(1);

	//set time
	m_activeGrid -> SetCellValue(newRow, 0, wxString::Format("%.3f", time));

	//set sensor values
	for(size_t i = 0; i < values.size(); ++i){
		m_activeGrid -> SetCellValue(newRow, i + 1, wxString::Format("%.2f", values[i]));
	}

	//m_activeGrid -> AutoSizeColumns();
}

void LiveDataWindow::clearAll()
{
	m_notebook -> DeleteAllPages();
	m_activeRun.reset();
	m_activeGrid = nullptr;
}
