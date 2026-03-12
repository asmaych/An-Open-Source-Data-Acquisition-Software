#include "MainFrame.h"
#include "ProjectPanel.h"
#include "SensorSelectionDialog.h"
#include <wx/wx.h>
#include <wx/artprov.h> //this one provides default system icons for toolbar items
#include "Sidebar.h"
#include "Events.h"
#include <wx/textdlg.h>


//Constructor for MainFrame - sets up the application shell
MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1100, 700))
{
	//set up wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	//make a status bar to display messages as "Project created.."
	CreateStatusBar();


		//--------------------------------------------------------------
        //      CREATE TOOLBAR:
        //--------------------------------------------------------------

	// The toolbar will appear at the top of the window.
	toolbar = new Toolbar(this);

	// Start/Stop experiment
	toolbar -> onStartStop = [this]() {
		auto* project = getCurrentProjectPanel();
		if(!project){
			wxMessageBox("Open a project first");
			return;
		}

		project -> toggleStartStop();
		toolbar -> setRunning(project -> isRunning());
	};

	//reset session
	toolbar -> onReset = [this]() {
                auto* project = getCurrentProjectPanel();
                if(!project){
                        wxMessageBox("Open a project first");
                        return;
                }

                project -> resetSessionData();
                toolbar -> setRunning(false);
        };

	//collect now
	toolbar -> onCollectNow = [this]() {
                auto* project = getCurrentProjectPanel();
                if(!project){
                        wxMessageBox("Open a project first");
                        return;
                }

                project -> collectCurrentValues();
        };

	//export
	toolbar -> onExport = [this]() {
                auto* project = getCurrentProjectPanel();
                if(!project){
                        wxMessageBox("Open a project first");
                        return;
                }

		wxCommandEvent evt;
                project -> exportSessions(evt);
        };

	//theme toggle switches between light and dark themes
	toolbar -> onToggleTheme = [this](){
				m_theme = (m_theme == Theme::Light)? Theme::Dark : Theme::Light;
				applyThemeToAll(m_theme);
				};

	//sidebar
	sidebar = new Sidebar(this);

	//noteBook
	m_notebook = new wxAuiNotebook(this, wxID_ANY);

	//dock sidebar to the left
	m_mgr.AddPane(sidebar, wxAuiPaneInfo().Left().BestSize(200, -1));

	//main workspace in the center
	m_mgr.AddPane(m_notebook, wxAuiPaneInfo().Center());

	//apply aui layout
	m_mgr.Update();

	//listen for "new project" events from sidebar/menu
	Bind(wxEVT_PROJECT_NEW, &MainFrame::onNewProject, this);

	//listen for theme toggle events from toolbar -> ProjectConfigDialog
	Bind(wxEVT_THEME_TOGGLE, &MainFrame::toggleTheme, this);

	//initialize the theme
	m_theme = Theme::Light;
	toggleTheme();
}

void MainFrame::toggleTheme(wxCommandEvent&) {
	m_theme = (m_theme == Theme::Light)? Theme::Dark : Theme::Light;
	applyThemeToAll(m_theme);
}

void MainFrame::toggleTheme() {
	m_theme = (m_theme == Theme::Light)? Theme::Dark : Theme::Light;
	applyThemeToAll(m_theme);
}

Theme MainFrame::getTheme() {
	return m_theme;
}

//getCurrentProjectPanel that returns the currently selected projectPanel or nullptr if no project is open
ProjectPanel* MainFrame::getCurrentProjectPanel()
{
	int selected = m_notebook -> GetSelection();
	if(selected == wxNOT_FOUND)
		return nullptr;

	return dynamic_cast<ProjectPanel*>(m_notebook -> GetPage(selected));
}

//onNewProject which creates a new project tab after prompting the user for a project name
void MainFrame::onNewProject(wxCommandEvent&)
{
	wxString name = wxGetTextFromUser("Project name", "New Project");

	if(name.IsEmpty())
		return;

	//create new project panel
	auto* panel = new ProjectPanel(m_notebook, name);
	panel -> setMainFrame(this);

	//add it as a new notebook tab
	m_notebook -> AddPage(panel, name, true);

	toolbar -> setCurrentProject(panel);

	wxLogStatus("Project created: %s", name);
}

void MainFrame::onOpenProject(wxCommandEvent& evt)
{
        //TODO implement this with sqlite

        //Display an informative message to users in status bar
        wxLogStatus("TODO - Opening an existing project...");
}


//applyThemeToAll which applies it to mainFrame, sidebar and all open projet panels
void MainFrame::applyThemeToAll(Theme theme)
{
	wxColour bg = (theme == Theme::Dark)? wxColour(30, 30, 30) : *wxWHITE;
	wxColour fg = (theme == Theme::Dark)? wxColour(220, 220, 220) : *wxBLACK;

	SetBackgroundColour(bg);
	SetForegroundColour(fg);

	toolbar -> applyTheme(theme);
	sidebar -> applyTheme(theme);

	//propagate theme to all open projects
	for(size_t i = 0; i < m_notebook -> GetPageCount(); ++i) {
		auto* panel = dynamic_cast<ProjectPanel*>(m_notebook -> GetPage(i));
		if(panel){
			panel -> applyTheme(theme);
		}
	}

	//force to repaint
	Refresh();
}
