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
	//call after is a method that lets you schedule a function or lambda to run later in the main GUI thread
	//here for instance: after adding new rows/data do not forget to autosizeColumns which makes sure that everything is visible
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

	//Auto scroll to the new row
	m_activeGrid -> MakeCellVisible(newRow, 0);
}

void LiveDataWindow::clearAll()
{
	m_notebook -> DeleteAllPages();
	m_activeRun.reset();
	m_activeGrid = nullptr;
}


void LiveDataWindow::applyTheme(Theme theme)
{
    	wxColour bg, fg;

    	if (theme == Theme::Dark) {
        	bg = wxColour(30, 30, 30);
        	fg = wxColour(220, 220, 220);
    	} else if(theme == Theme::Light) {
        	bg = *wxWHITE;
        	fg = *wxBLACK;
    	}

    	//set background for the notebook itself
    	m_notebook->SetBackgroundColour(bg);
    	m_notebook->SetForegroundColour(fg);

    	//iterate through all pages (grids) and apply theme
    	for (size_t i = 0; i < m_notebook->GetPageCount(); ++i) {
        	wxWindow* page = m_notebook->GetPage(i);
        	if (wxGrid* grid = dynamic_cast<wxGrid*>(page)) {
            		grid->SetBackgroundColour(bg);
            		grid->SetForegroundColour(fg);
            		grid->SetDefaultCellBackgroundColour(bg);
            		grid->SetDefaultCellTextColour(fg);
            		grid->SetLabelBackgroundColour(bg);
            		grid->SetLabelTextColour(fg);

            		grid->ClearSelection();
            		grid->ForceRefresh();
        	}
    	}

    	Layout();
    	Refresh();
}


/*
        replays a complete historical run into the notebook as a read only tab
*/
void LiveDataWindow::addHistoricalRun(std::shared_ptr<Run> run, const std::vector<std::unique_ptr<Sensor>>& sensors)
{
        if(!run || run->getFrames().empty())
                return;

        //build sensor name list for column headers
        std::vector<std::string> names;
        for(auto& s : sensors)
                names.push_back(s->getName());

        //create the grid for this run
        wxGrid* grid = new wxGrid(m_notebook, wxID_ANY);
        grid->SetMinSize(wxSize(200, 150));
        grid->CreateGrid(0, static_cast<int>(names.size() + 1)); // +1 for time
        grid->EnableEditing(false);
        grid->SetRowLabelSize(40);

        wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        grid->SetDefaultCellFont(font);
        grid->SetLabelFont(font);

        grid->SetColLabelValue(0, "Time (s)");
        for(size_t i = 0; i < names.size(); ++i)
                grid->SetColLabelValue(i + 1, names[i]);

        //add tab to notebook — NOT set as m_activeGrid
        wxString title = wxString::Format("Run %zu", run->getRunNumber());
        m_notebook->AddPage(grid, title, false); // false = don't select this tab

        //fill every row directly into this grid
        const auto& times  = run->getTimes();
        const auto& frames = run->getFrames();

        for(size_t i = 0; i < times.size(); ++i){
                int row = grid -> GetNumberRows();
                grid->AppendRows(1);
                grid->SetCellValue(row, 0, wxString::Format("%.3f", times[i]));

                for(size_t j = 0; j < frames[i].size(); ++j)
                        grid->SetCellValue(row, j + 1, wxString::Format("%.2f", frames[i][j]));
        }

        //autosize after all data is written
        CallAfter([grid](){
                if(grid)
                        grid -> AutoSizeColumns();
        });
}
