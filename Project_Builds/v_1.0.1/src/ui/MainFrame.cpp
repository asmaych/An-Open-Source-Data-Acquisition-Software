#include "MainFrame.h"
#include "ProjectPanel.h"
#include <wx/wx.h>

#include "Sidebar.h"
#include "Events.h"


MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title)
{
	//set the class-wide wxAuiManager to manage this window:
	m_mgr.SetManagedWindow(this);

	//make a status bar to display messages:
	CreateStatusBar();

	//--------------------------------------------------------------
	//	//INSTANTIATING OBJECTS:
	//--------------------------------------------------------------
	
	//SIDEBAR
	sidebar = new Sidebar(this);

	//PROJECT SPACE
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
			wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT);
	

	//--------------------------------------------------------------
	//ADDING OBJECTS TO WINDOW MANAGER
	//--------------------------------------------------------------
	
	//SIDEBAR:
	m_mgr.AddPane(sidebar,
			wxAuiPaneInfo()
			.Name("sidebar")
			.Left()
			.Caption("Sidebar")
			.BestSize(250, -1));
	
	//PROJECT SPACE:
	m_mgr.AddPane(m_notebook,
			wxAuiPaneInfo()
			.Center()
			.Caption("Projects")
			.CloseButton(false));


	//commit the items to the manager:
	m_mgr.Update();

	//binding the sidebar "Create New Project" event to a handler
	Bind(wxEVT_PROJECT_NEW, &MainFrame::onNewProject, this);


	//wxButton* button = new wxButton(panel, wxID_ANY, "New Project", wxDefaultPosition, wxSize(200,50));
	//button->Bind(wxEVT_BUTTON, &MainFrame::onCreateProject, this);

}

void MainFrame::onNewProject(wxCommandEvent& evt)
{
	//Display an informative message to users
	wxLogStatus("Creating new project");


	//give a name for the new project:
	wxString projectName = wxString::Format("Project %d", ++m_project_count);

	//now call the helper function to make the new project
	addProjectTab(projectName);
}

void MainFrame::addProjectTab(const wxString& Pname)
{
	//make a temporary (local scope) pointer to a newly allocated
	//ProjectPanel, and give it to the wxAuiNotebook
	ProjectPanel* panel = new ProjectPanel(m_notebook, Pname);

	//the true flag switches focus to new tab
	m_notebook->AddPage(panel, Pname, true);	
}

