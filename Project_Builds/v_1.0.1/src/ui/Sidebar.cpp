#include <wx/wx.h>
#include "Sidebar.h"
#include "Events.h"

/*  SIdebar is a vertical panel with buttons for common project actions as New Project, Load Project (we can add openSensor later).
    By clicking "New Project", we post an event, thus MainFrame creates a tab.

*/

//define the custom event:
wxDEFINE_EVENT(wxEVT_PROJECT_NEW, wxCommandEvent);

Sidebar::Sidebar(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
	//choose vertical alignment so that items stack down the sidebar
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	//make a button for creating project:
	wxButton* new_project_button = new wxButton(this, wxID_ANY, "New Project");

	//make a button for loading a project (opening an existing one):
	//TODO: implement file open later
	wxButton* load_project_button = new wxButton(this, wxID_ANY, "Load Project");

	//now add both buttons to the sizer
	sizer->Add(new_project_button, 0, wxALL | wxEXPAND, 10);
	sizer->Add(load_project_button, 0, wxALL | wxEXPAND, 10);
	
	//setSizer for layout
	SetSizer(sizer);

	//EVENT TRIGGERS: clicking "New Project" triggers custom event wxEVT_PROJECT_NEW
	new_project_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
		{
			wxCommandEvent evt(wxEVT_PROJECT_NEW);
			evt.SetEventObject(this);
			wxPostEvent(GetParent(), evt); //post to MainFrame
		});

	//clicking "Open Project" 
	load_project_button ->Bind(wxEVT_BUTTON,[this](wxCommandEvent&)
 		{
		wxLogStatus("Open Project Button clicked (TODO)");
	
		});
}
