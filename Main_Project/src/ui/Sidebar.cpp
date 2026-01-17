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
	new_project_button = new wxButton(this, wxID_ANY, "New Project");

	//Bind button click event to Sidebar::OnNewProject
	new_project_button -> Bind(wxEVT_BUTTON, &Sidebar::OnNewProject, this);

	//Add button to sizer with some spacing
	sizer -> Add(new_project_button, 0, wxEXPAND | wxALL, 5);

	//now we do the same for LoadProject
        load_project_button = new wxButton(this, wxID_ANY, "Load Project");

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

void Sidebar::applyTheme(Theme theme)
{
	wxColour bg, fg, btnBg;

	if(theme == Theme::Dark){
		bg = wxColour(30,30,30);
		fg = wxColour(220,220,220);
		btnBg = wxColour(60,60,60);
	} else {
		bg = *wxWHITE;
		fg = *wxBLACK;
		btnBg = wxColour(240, 240, 240);
	}

	//panel background
	SetBackgroundColour(bg);
	SetForegroundColour(fg);

	//buttons
	if(new_project_button){
		new_project_button -> SetBackgroundColour(btnBg);
		new_project_button -> SetForegroundColour(fg);
	}

	if(load_project_button){
		load_project_button -> SetBackgroundColour(btnBg);
		load_project_button -> SetForegroundColour(fg);
	}

	Refresh();
}

