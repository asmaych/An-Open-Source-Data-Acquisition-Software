#include <wx/wx.h>
#include "Sidebar.h"
#include "Events.h"
#include "MainFrame.h"

/*  SIdebar is a vertical panel with buttons for common project actions as New Project, Load Project.
    By clicking "New Project", we post an event, thus MainFrame creates a tab.

*/

Sidebar::Sidebar(MainFrame* parent) 
	: wxPanel(parent, wxID_ANY), m_parent(parent)
{
	//Create vertical box sizer to layout buttons vertically
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	//Create NewProject
	wxButton* new_project_button = new wxButton(this, wxID_ANY, "New Project");

	//Bind button click event to Sidebar::OnNewProject
	new_project_button -> Bind(wxEVT_BUTTON, &Sidebar::OnNewProject, this);

	//Add button to sizer with some spacing
	sizer -> Add(new_project_button, 0, wxEXPAND | wxALL, 5);

	//now we do the same for LoadProject
        wxButton* load_project_button = new wxButton(this, wxID_ANY, "Load Project");

        load_project_button -> Bind(wxEVT_BUTTON, &Sidebar::OnLoadProject, this);

        sizer -> Add(load_project_button, 0, wxEXPAND | wxALL, 5);
	//setSizer for layout
	SetSizerAndFit(sizer);
}
/* Event handler for both "New Project" button & "Load Project" button
   when clicking NewProject, sidebar calls MainFrame's function to create a new project.
   when clicking LOadProject, the sidebar will call the MainFrame's function to load an existing project.
*/

void Sidebar::OnNewProject(wxCommandEvent& evt){
	if(m_parent){ //safety check
		m_parent -> onNewProject(evt); //calls mainFRame handler
	}
}

void Sidebar::OnLoadProject(wxCommandEvent& evt){
	if(m_parent){ 
		m_parent -> onOpenProject(evt);
	}
}
