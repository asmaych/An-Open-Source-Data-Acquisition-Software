#include "MainFrame.h"
#include "ProjectPanel.h"
#include <wx/wx.h>
#include <wx/artprov.h> //this one provides default system icons for toolbar items
#include "Sidebar.h"
#include "Events.h"

//THis an eve,t table and it must be outside any class or function


//Constructor for MainFRame - sets up the application shell
MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title)
{
	//set the class-wide wxAuiManager to manage this window:
	m_mgr.SetManagedWindow(this);

	//make a status bar to display messages as "Project created.."
	CreateStatusBar();


	///--------------------------------------------------------------
        //      //CREATE TOOLBAR:
        //--------------------------------------------------------------

	/* The toolbar will appear at the top of the window.
	   It has quick access buttons like open project, create project...
	   wxTB_HORIZONTAL -> makes the toolbar horizontal
	   wxNO_BORDER -> removes the 3D border aound it
	   wxTB_FLAT -> gives a modern flat look
	   wxTB_TEXT -> displays buttons labels under their icons
	*/
	toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);

	/* Now i will add a nexProject button with a default system icon
	   Addtool (uniqueID for the button, text shown under the icon, default "new file" icon, tooltip when hovered (when u put the mouse over the icon 
	   u should see create new project);
	*/

	toolbar -> AddTool(ID_NewProject, "New Project", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR), "Create New Project");

	//Add open Project button
	toolbar -> AddTool(ID_OpenProject, "Open Project", wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR), "Open an existing Project");

	//Add sensor
	toolbar -> AddTool(ID_AddSensor, "Add Sensor", wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR), "Add a new Sensor");

	//Remove sensor
	toolbar -> AddTool(ID_RemoveSensor, "Remove Sensor", wxArtProvider::GetBitmap(wxART_MINUS, wxART_TOOLBAR), "Remove the selected sensor");

	//Finalize and display our toolbar YAAAYYYY!!!!
	toolbar->Realize();  //a method that takes no parameters and should be called in each time something is modififed in the toolbar	

	//--------------------------------------------------------------
	//	//INSTANTIATING OBJECTS:
	//--------------------------------------------------------------
	
	//SIDEBAR on the left with buttons like "New Project, "Load Project"...
	sidebar = new Sidebar(this);

	//PROJECT SPACE - Notebook (tab control) in the center to hold multiple ProjectPanels
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
			wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT);
	

	//--------------------------------------------------------------
	//ADDING OBJECTS TO WINDOW MANAGER
	//--------------------------------------------------------------
	
	//SIDEBAR - sidebar goes to the left
	m_mgr.AddPane(sidebar,
			wxAuiPaneInfo()
			.Name("sidebar")
			.Left()
			.Caption("Sidebar")
			.BestSize(250, -1)); //width 250 while height is flexible
	
	//PROJECT SPACE = notebook (project panels) goes to the center
	m_mgr.AddPane(m_notebook,
			wxAuiPaneInfo()
			.Center()
			.Caption("Projects")
			.CloseButton(false)); //no close button for notebook itself


	//commit the items to the manager:
	m_mgr.Update();

	//binding the sidebar "Create New Project" event to a handler 
	Bind(wxEVT_PROJECT_NEW, &MainFrame::onNewProject, this);

	//when the user clicks "new project" in the toolbar
	Bind(wxEVT_TOOL, &MainFrame::onNewProject, this, wxID_NEW);
	
	//handle the open project button by displaying a message (temporary) TODO
	Bind(wxEVT_TOOL, [&](wxCommandEvent& evt) {
		wxLogStatus("Open Project clicked (TODO)");
	}, wxID_OPEN);
	Bind(wxEVT_TOOL, &MainFrame::onNewProject, this, ID_NewProject);
	Bind(wxEVT_TOOL, &MainFrame::onOpenProject, this, ID_OpenProject);
	Bind(wxEVT_TOOL, &MainFrame::onAddSensor, this, ID_AddSensor);
	Bind(wxEVT_TOOL, &MainFrame::onRemoveSensor, this, ID_RemoveSensor);

	//wxButton* button = new wxButton(panel, wxID_ANY, "New Project", wxDefaultPosition, wxSize(200,50));
	//button->Bind(wxEVT_BUTTON, &MainFrame::onCreateProject, this);

}

//event handler for creating new project
void MainFrame::onNewProject(wxCommandEvent& evt)
{
	//Display an informative message to users in status bar
	wxLogStatus("Creating new project");


	//give a name for the new project:
	wxString projectName = wxString::Format("Project %d", ++m_project_count);

	//now call the helper function to make the new project
	addProjectTab(projectName);
}

//Helper function to add a projectPanel as a new tab in notebook
void MainFrame::addProjectTab(const wxString& Pname)
{
	//make a temporary (local scope) pointer to a newly allocated
	//ProjectPanel, and give it to the wxAuiNotebook
	ProjectPanel* panel = new ProjectPanel(m_notebook, Pname);

	//Add panel as a new tab, the true flag switches focus to new tab
	m_notebook->AddPage(panel, Pname, true);	
}

//event handler for creating new project
void MainFrame::onOpenProject(wxCommandEvent& evt)
{
        //Display an informative message to users in status bar
        wxLogStatus("Opening an existing project...");
}

//event handler for adding a sensor
void MainFrame::onAddSensor(wxCommandEvent& evt)
{
	//Display an informative message to users in status bar
	wxLogStatus("Adding a sensor...");
}

//event handler for creating new project
void MainFrame::onRemoveSensor(wxCommandEvent& evt)
{
        //Display an informative message to users in status bar
        wxLogStatus("Removing a sensor...");
}


