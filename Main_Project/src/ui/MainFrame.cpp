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
	//open db at software start
	if(!m_DB.open("../daq_database.db")){
		wxMessageBox("Failed to open database!", "Database Error");
	}

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
	m_mgr.AddPane(sidebar, wxAuiPaneInfo().Left().BestSize(200, -1).CloseButton(false));

	//main workspace in the center
	m_mgr.AddPane(m_notebook, wxAuiPaneInfo().Center().CloseButton(false));

	//apply aui layout
	m_mgr.Update();

	//listen for "new project" events from sidebar/menu
	Bind(wxEVT_PROJECT_NEW, &MainFrame::onNewProject, this);

	m_notebook -> Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &MainFrame::onPageClose, this);

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


//destructor to ensure that the db closes properly
MainFrame::~MainFrame()
{
	m_DB.close();
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

	int projectID = -1;

    	//ask user if they want to save this project
        int answer = wxMessageBox("Do you want to save this project in the database?", "Save Project", wxYES_NO | wxICON_QUESTION);

	 if(answer == wxYES){
        	//create the project in db and get its ID
        	projectID = m_DB.createProject(name.ToStdString());

        	if(projectID < 0){
            		wxMessageBox("Failed to save project in database!", "Error");
        	}
        	else{
            		wxLogStatus("Project saved in DB with ID: %d", projectID);
        	}
    	}

    	//create the project panel
        auto* panel = new ProjectPanel(m_notebook, name, &m_DB);

	//store db project ID in panel
    	if(projectID >= 0){
        	panel -> setProjectId(projectID);
		panel -> setSaveProject(true);
	}
	else{
		panel -> setProjectId(-1);
		panel -> setSaveProject(false);
	}

    	panel -> setMainFrame(this);

	//add it as a new notebook tab
	m_notebook -> AddPage(panel, name, true);

	//tell toolbar which project is current
	toolbar -> setCurrentProject(panel);

	wxLogStatus("Project created: %s", name);
}

void MainFrame::onOpenProject(wxCommandEvent& evt)
{
        //create a vector to store project names
	std::vector<std::string> projects;

	//ask the db manager to load all project names from db to the vector
	m_DB.loadProjects(projects);

	//if the vector is empty, display a message
	if(projects.empty()){
		wxMessageBox("No projects found in database!");
		return;
	}

	//we create a list of choices for the user
	wxArrayString choices;

	for(auto& project : projects)
		choices.Add(project);

	//create the dialog to display the choices
	wxSingleChoiceDialog dlg(this, "Select project", "Open project", choices);

	//if the user presses cancel, forget about it
	if(dlg.ShowModal() != wxID_OK)
		return;

	//get the selected project name
	wxString name = dlg.GetStringSelection();

	//the projects are saved by id
	int projectID = m_DB.getProjectID(name.ToStdString());

	if(projectID < 0){
		wxMessageBox("Could not find project in Database!", "Error");
		return;
	}

	//create a new project panel where m_notebook is the parent notebook, name is the project name, and pointer for db manager
	auto* panel = new ProjectPanel(m_notebook, name, &m_DB);

	//store the project id inside the panel so the panel knows which project it belongs to
	panel -> setProjectId(projectID);
	panel -> setSaveProject(true);

	panel -> setMainFrame(this);

	//load all project data from the db
	panel -> loadProjectFromDatabase();

	//add the new panel as a tab in the notebook
	m_notebook -> AddPage(panel, name, true);

	//inform the toolbar which project panel is currently active
	toolbar -> setCurrentProject(panel);

	//display a log status indicating what project was loaded
	wxLogStatus("Project loaded: %s", name);
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


//this fires before the panel is destroyed, so toggleStartStop() runs cleanly, it sends stop to the esp32, calls stopRun which saves
//the ui state to DB, and clears m_isRunning
void MainFrame::onPageClose(wxAuiNotebookEvent& evt)
{
    	//get the panel that is about to be closed
    	int pageIndex = evt.GetSelection();
    	if(pageIndex == wxNOT_FOUND){
        	evt.Skip();
        	return;
    	}

    	ProjectPanel* panel = dynamic_cast<ProjectPanel*>(m_notebook->GetPage(pageIndex));

    	if(!panel){
        	evt.Skip();
        	return;
    	}

    	//if an experiment is running, stop it cleanly before closing, this ensures the esp32 is told to stop streaming, thus ui saved cleanly
        if(panel -> isRunning()){
        	//tell the microcontroller to stop streaming
        	panel -> toggleStartStop();

		toolbar -> setRunning(false);

         	std::cout << "Run stopped on project close\n";
    	}

    	//update the toolbar if the closed tab was the active one
    	toolbar -> setCurrentProject(nullptr);

    	//allow the close to proceed
    	evt.Skip();
}
