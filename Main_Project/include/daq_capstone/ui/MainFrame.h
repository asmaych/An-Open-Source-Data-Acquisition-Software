#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "Sidebar.h"
#include "ProjectPanel.h"
#include "HandshakeDialog.h"

/* MainFrame class is the main application window, it contains:
   A toolbar with features as Start, Stop, COllect, Sensor, Graph, that are disabled unless a project is open + actions as start/
   stop/collect/graph won't run unless a sensor is selected.
   A sidebar on the left with New Project & Load Project.
   A project space: wxAUINotebook with ProjectPanel tabs.
*/
class ProjectPanel;
class Sidebar;

class MainFrame : public wxFrame
{
	public:
		
		enum ToolbarIDs {
			ID_Start = wxID_HIGHEST + 1,
			ID_Stop,
			ID_Sensor,
			ID_Graph,
			ID_Collect
		};
		MainFrame(const wxString& title);
		//----------------------------------------------------
                //      PROJECT ACTIONS
                //----------------------------------------------------
                void onNewProject(wxCommandEvent& evt);
                void onOpenProject(wxCommandEvent& evt);


	private:
		//----------------------------------------------------
                //      HELPER: returns currently selected ProjectPanel
                //----------------------------------------------------
		ProjectPanel* getCurrentProjectPanel();

		//----------------------------------------------------
		//	TOOLBAR EVENT HANDLERS
		//----------------------------------------------------
		void onStart(wxCommandEvent& evt);
		void onStop(wxCommandEvent& evt);
		void onCollect(wxCommandEvent& evt);
		void onGraph(wxCommandEvent& evt);
		void onSensor(wxCommandEvent& evt);
        
		//----------------------------------------------------
		//	CLASS ATTRIBUTES
		//----------------------------------------------------

		//declare a wxAUI object for use across the class:
		wxAuiManager m_mgr;

		//declare a notebook for project tabs
		wxAuiNotebook* m_notebook;

		//declare the sidebar
		Sidebar* sidebar = nullptr;

		//Toolbar pointer (new) = stores a pointer to the toobar so u can access it later 
		wxToolBar* toolbar = nullptr;

		//ProjectCounter for naming new projects automatically
		int m_project_count = 0;

};
