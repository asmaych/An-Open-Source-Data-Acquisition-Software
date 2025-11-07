#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "Sidebar.h"
#include "ProjectPanel.h"

class MainFrame : public wxFrame
{
	public:
		MainFrame(const wxString& title);
	
	private:
		//----------------------------------------------------
		//HELPER FUNCTIONS
		//----------------------------------------------------
		void addProjectTab(const wxString& Pname);

		//----------------------------------------------------
		//EVENT HANDLERS
		//----------------------------------------------------
		void onNewProject(wxCommandEvent& evt);
		void onOpenProject(wxCommandEvent& evt);
		void onAddSensor(wxCommandEvent& evt);
		void onRemoveSensor(wxCommandEvent& evt);
	
		//----------------------------------------------------
		//CLASS ATTRIBUTES
		//----------------------------------------------------

		//declare a wxAUI object for use across the class:
		wxAuiManager m_mgr;

		//declare a notebook for project tabs
		wxAuiNotebook* m_notebook = nullptr;
		//and a counter to track projects
		int m_project_count = 0;

		//declare the sidebar
		Sidebar* sidebar = nullptr;

		//Toolbar pointer (new) = stores a pointer to the toobar so u can access it later 
		wxToolBar* toolbar = nullptr;

		//command IDs for toolbar buttons, wxwidgets needs unique IDs to link clicks to handlers
		enum {
			ID_NewProject = wxID_HIGHEST + 1,
			ID_OpenProject,
			ID_AddSensor,
			ID_RemoveSensor
		     };

		//wxDECLARE_EVENT_TABLE(); //connects menu/toolbar events to handlers (tells wxwidget that this class will have 
					// an event table in the .cpp file where we bind button clicks

};
