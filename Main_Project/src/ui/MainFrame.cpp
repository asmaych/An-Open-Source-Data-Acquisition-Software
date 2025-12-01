#include "MainFrame.h"
#include "ProjectPanel.h"
#include <wx/wx.h>
#include <wx/artprov.h> //this one provides default system icons for toolbar items
#include "Sidebar.h"
#include "Events.h"
#include <wx/textdlg.h>


//Constructor for MainFrame - sets up the application shell
MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 600))
{
	//set up wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	//make a status bar to display messages as "Project created.."
	CreateStatusBar();


	///--------------------------------------------------------------
        //      CREATE TOOLBAR:
        //--------------------------------------------------------------

	/* The toolbar will appear at the top of the window.
	   It has buttons like start, stop...
	   wxTB_HORIZONTAL -> makes the toolbar horizontal
	   wxNO_BORDER -> removes the 3D border aound it
	   wxTB_FLAT -> gives a modern flat look
	   wxTB_TEXT -> displays buttons labels under their icons
	*/
	toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);

	/* Now i will add a start, stop, sensor.. buttons with a default system icon
	   Addtool (any ID, text shown under the icon, default "new file" icon, tooltip 
	   when hovered (when u put the mouse over the icon u should see create new project);
	*/

	toolbar -> AddTool(ID_StartToggle,
		  	"Start",
		       	wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_TOOLBAR),
		       	"Start/Stop experiment");

	toolbar -> AddTool(ID_Reset,
		       	"Reset",
		       	wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR),
		       	"Reset sessions");

	toolbar -> AddTool(ID_Collect,
		       	"Collect",
		       	wxArtProvider::GetBitmap(wxART_REPORT_VIEW, wxART_TOOLBAR),
		       	"Collect data");

	toolbar -> AddTool(ID_Collect_Current,
                        "CollectNow",
                        wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR),
                        "Collect last values");

	toolbar -> AddTool(ID_Graph,
		       	"Graph",
		       	wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_TOOLBAR),
		       	"Graph collected data");
	
	toolbar -> AddTool(ID_Sensor,
		       	"Sensors",
		       	wxArtProvider::GetBitmap(wxART_TIP, wxART_TOOLBAR),
		       	"Manage sensors");

	toolbar -> AddTool(ID_Export,
                        "Export",
                        wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR),
                        "Export sessions");


	//Finalize and display our toolbar YAAAYYYY!!!!
	//Realize() a method that takes no parameters and should be called in 
	//each time something is modififed in the toolbar
	toolbar->Realize();


	//--------------------------------------------------------------
	//	INSTANTIATING OBJECTS:
	//--------------------------------------------------------------
	
	//SIDEBAR on the left with buttons like "New Project, "Load Project"...
	sidebar = new Sidebar(this);

	//PROJECT SPACE - Notebook (tab control) in the center to hold multiple ProjectPanels
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
			wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT);
	

	//--------------------------------------------------------------
	//       ADDING OBJECTS TO WINDOW MANAGER
	//--------------------------------------------------------------
	
	//SIDEBAR - sidebar goes to the left
	m_mgr.AddPane(sidebar,
			wxAuiPaneInfo()
			.Name("sidebar")
			.Left()
			.Caption("Sidebar")
			.BestSize(200, -1)); //width 200 while height is flexible
	
	//PROJECT SPACE = notebook (project panels) goes to the center
	m_mgr.AddPane(m_notebook,
			wxAuiPaneInfo()
			.Center()
			.Caption("Projects")
			.CloseButton(false)); //no close button for notebook itself


	//commit the items to the manager:
	m_mgr.Update();


        //--------------------------------------------------------------
        //       BINDING EVENTS
        //--------------------------------------------------------------
        
	Bind(wxEVT_TOOL, &MainFrame::onStartToggle, this, ID_StartToggle);
	Bind(wxEVT_TOOL, &MainFrame::onReset, this, ID_Reset);
        Bind(wxEVT_TOOL, &MainFrame::onCollect, this, ID_Collect);
	Bind(wxEVT_TOOL, &MainFrame::onCollectCurrent, this, ID_Collect_Current);
        Bind(wxEVT_TOOL, &MainFrame::onGraph, this, ID_Graph);
	Bind(wxEVT_TOOL, &MainFrame::onSensor, this, ID_Sensor);
        Bind(wxEVT_TOOL, &MainFrame::onExport, this, ID_Export);


	//Bind custom project events
        Bind(wxEVT_PROJECT_NEW, &MainFrame::onNewProject, this);
}

//--------------------------------------------------------------
//       HELPERS
//--------------------------------------------------------------

// Returns a pointer to the currently selected ProjectPanel - nullptr if none
ProjectPanel* MainFrame::getCurrentProjectPanel()
{
	//returns the index of the currently selected project tab 
	int selected = m_notebook -> GetSelection();

	//immediately return null pointer if there is no selection open
	if (selected == wxNOT_FOUND)
		return nullptr;

	/* this line returns the currently selected project panel.
	   dynamic_cast<ProjectPanel*> tries to safely cast the wxwindow* pointer to a ProjectPanel*.
	   if the page is a projectPanel, the cast succeeds, else it returns nullptr.
	   WHY? this avoids runtime errors when working with multiple panels in the notebook.
	*/
	return dynamic_cast<ProjectPanel*>(m_notebook -> GetPage(selected));
}

//--------------------------------------------------------------
//       EVENT HANDLERS 
//--------------------------------------------------------------

void MainFrame::onNewProject(wxCommandEvent& evt)
{
	//Display an informative message to users in status bar
	wxLogStatus("Creating new project");


	//give a name for the new project:
	wxString projectName = wxGetTextFromUser(
			"Enter a name for the new project:",
			"New Project",
			"",
			this);

	//do not create a project if the user enters an empty string
	if (projectName.IsEmpty())
	{
		return;
	}

	//create a new ProjectPanel and add it to the notebook
	ProjectPanel* panel = new ProjectPanel(m_notebook, projectName);

	panel -> setMainFrame(this);

        m_notebook->AddPage(panel, projectName, true);

	//Disply an info message to users in the status bar
	wxLogStatus("New Project created: %s", projectName);
}


void MainFrame::onOpenProject(wxCommandEvent& evt)
{
	//TODO implement this with sqlite

        //Display an informative message to users in status bar
        wxLogStatus("TODO - Opening an existing project...");
}

void MainFrame::onStartToggle(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->toggleStartStop(); // implemented in ProjectPanel
}

void MainFrame::onReset(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->resetSessionData();
}

void MainFrame::onCollect(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->collectContinuous();
}

void MainFrame::onCollectCurrent(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->collectCurrentValues();
}

void MainFrame::onGraph(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->graphSelectedSensor();
}

/*void MainFrame::onSensor(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->openSensorPanel();
}
*/
void MainFrame::onExport(wxCommandEvent&)
{
    ProjectPanel* p = getCurrentProjectPanel();
    if (!p) { wxMessageBox("Please open or load a project first!", "Warning"); return; }
    p->exportSessions();
}

void MainFrame::onSensor(wxCommandEvent& evt)
{
        ProjectPanel* project = getCurrentProjectPanel();
        if(!project){
                wxMessageBox("Please open or load a project first!", "Warning", wxICON_WARNING);
                return;
        }
        project ->onSensors();
}

void MainFrame::setStartToggleToStop()
{
	if(!toolbar){
		return;
	}

	//find the position of the tool before deleting it
	int position = toolbar -> GetToolPos(ID_StartToggle);

	//remove old tool
	toolbar -> DeleteTool(ID_StartToggle);

	//insert the new tool at the same index
	toolbar -> InsertTool(position, ID_StartToggle, "Stop", wxArtProvider::GetBitmap(wxART_STOP, wxART_TOOLBAR), 
                              wxNullBitmap, wxITEM_NORMAL, "Stop experiment", "");

	toolbar -> Realize();
}

void MainFrame::setStartToggleToStart()
{
	if(!toolbar){
		return;
	}

	int position = toolbar -> GetToolPos(ID_StartToggle);

	toolbar -> DeleteTool(ID_StartToggle);

	toolbar -> InsertTool(position, ID_StartToggle,  "Start", wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_TOOLBAR), 
                              wxNullBitmap, wxITEM_NORMAL, "Start experiment", "");

	toolbar -> Realize();
}





