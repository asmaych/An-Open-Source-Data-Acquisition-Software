#include "Toolbar.h"
#include <wx/aui/aui.h>
#include <wx/artprov.h>

/* Toolbar is a class that encapsulates the main toolbar buttons and features through creating them and managing their
   visual state (toggle)
	- start/stop to start and stop an experiment
	- reset to clear all the runs meaning the live display thus the live graphing
	- Collect now which creates a table of time and values that appends a row whenever collects on demand is pressed
	- Graph which live graphs the runs with different colors and labeled by names
	- export that exports the data to csv automatically and the graph to png
	- theme that changes the theme of Side/Toolbar and mainFrame from dark to light and vice versa
*/

Toolbar::Toolbar(wxFrame* parent)
{
	/* Flags:
		- wxTB_HORIZONTAL : horizontal toolbar
		- wxNO_BORDER : cleaner, modern look
		- wxTB_FLAT : flat buttons (no 3D effect)
		- wxTB_TEXT : show text labels under icons
		- wxTB_NODIVIDER : no separators between tools
	*/

	m_toolbar = parent -> CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER);

	m_toolbar -> SetToolBitmapSize(wxSize(32,32));

	m_toolbar -> AddTool(ID_StartStop, "Start", wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_TOOLBAR),
			    "Start/Stop experiment");

	//create a timer label and add it next to Start button
	//m_timerDisplay = new wxStaticText(m_toolbar, wxID_ANY, "00:00:00", wxDefaultPosition,
	//				wxDefaultSize, wxALIGN_CENTER | wxST_NO_AUTORESIZE);

	m_timerDisplay = new wxStaticText(m_toolbar, wxID_ANY, "00:00:00");
	//use AddControl to add the label to the toolbar
	m_toolbar->AddControl(m_timerDisplay);

	m_toolbar -> AddTool(ID_Reset, "Reset", wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR),
                            "Reset session");

	m_toolbar -> AddTool(ID_CollectNow, "Collect Now", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR),
                            "Collect current values");

	m_toolbar -> AddTool(ID_Graph, "Graph", wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_TOOLBAR),
                            "Graph Data");

	m_toolbar -> AddTool(ID_Export, "Export", wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR),
                            "Export data");

	m_toolbar -> AddTool(ID_Theme, "Theme", wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_TOOLBAR),
                            "Dark/Light theme");


	//finalize toolbar creation
	m_toolbar -> Realize();

	//bind toolbar button events to callbacks
	bindEvents(parent);
}

void Toolbar::bindEvents(wxFrame* parent)
{
	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
		if(onStartStop) onStartStop();
		}, ID_StartStop);

	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
                if(onReset) onReset();
                }, ID_Reset);

	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
                if(onCollectNow) onCollectNow();
                }, ID_CollectNow);

	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
                if(onGraph) onGraph();
                }, ID_Graph);

	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
                if(onExport) onExport();
                }, ID_Export);

	parent -> Bind(wxEVT_TOOL, [this](wxCommandEvent&) {
                if(onToggleTheme) onToggleTheme();
                }, ID_Theme);
}

// setRunning updates the start/stop button based on experiment state.
void Toolbar::setRunning(bool running)
{
	//avoid unnecessary UI rebuilds
	if(m_isRunning == running)
		return;

	m_isRunning = running;

	//remember position so toolbar layout stays the same
	int position = m_toolbar -> GetToolPos(ID_StartStop);

	//remove existing Start/Stop button
	m_toolbar -> DeleteTool(ID_StartStop);

	//insert appropriate version
	if(running){
		m_toolbar -> InsertTool(position, ID_StartStop, "Stop",
					wxArtProvider::GetBitmap(wxART_STOP, wxART_TOOLBAR), wxNullBitmap,
					wxITEM_NORMAL, "Stop experiment");
		}
	else {
		m_toolbar -> InsertTool(position, ID_StartStop, "Start", 
                                        wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_TOOLBAR), wxNullBitmap,
                                        wxITEM_NORMAL, "Start experiment");
	}

	//rebuild toolbar visuals
	m_toolbar -> Realize();
}

//applyTheme applies light or dark theme colors to the toolbar
void Toolbar::applyTheme(Theme theme)
{
	wxColour bg, fg;

	if(theme == Theme::Dark) {
		bg = wxColour(30, 30, 30);
		fg = wxColour(220, 220, 220);
	} else {
		bg = *wxWHITE;
		fg = *wxBLACK;
	}

	m_toolbar -> SetBackgroundColour(bg);
	m_toolbar -> SetForegroundColour(fg);
	m_toolbar -> Refresh();
}

//getter
wxToolBar* Toolbar::get()
{
	return m_toolbar;
}
