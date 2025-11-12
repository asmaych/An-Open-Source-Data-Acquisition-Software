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

};
