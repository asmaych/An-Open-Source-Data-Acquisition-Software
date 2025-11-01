#include <wx/wx.h>
#include "Sidebar.h"
#include "Events.h"

//define the custom event:
wxDEFINE_EVENT(wxEVT_PROJECT_NEW, wxCommandEvent);

Sidebar::Sidebar(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
	//choose vertical alignment so that items stack down the sidebar
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	//make a button for creating project:
	wxButton* new_project_button = new wxButton(this, wxID_ANY, "New Project");

	//make a button for loading a project:
	//TODO: implement functionality
	wxButton* load_project_button = new wxButton(this, wxID_ANY, "Load Project");

	//now add both buttons to the sizer
	sizer->Add(new_project_button, 0, wxALL | wxEXPAND, 10);
	sizer->Add(load_project_button, 0, wxALL | wxEXPAND, 10);

	SetSizer(sizer);

	//EVENT TRIGGERS:
	new_project_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
		{
			wxCommandEvent evt(wxEVT_PROJECT_NEW);
			evt.SetEventObject(this);
			wxPostEvent(GetParent(), evt);		
		});
	
}
