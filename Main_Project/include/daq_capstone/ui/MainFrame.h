#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "Sidebar.h"
#include "ProjectPanel.h"
#include "HandshakeDialog.h"
#include "Theme.h"
#include "Toolbar.h"

/* MainFrame class is the main application window, it contains:
   A toolbar with features like Start, Stop, Collect, Sensor and Graph, 
   that are disabled unless a project is open, and where actions like start/
   stop/collect/graph won't run unless a sensor is selected.
   A sidebar is  on the left with New Project & Load Project.
   A project space uses wxAUINotebook with ProjectPanel tabs.
*/
class ProjectPanel;
class Sidebar;
class Toolbar;

class MainFrame : public wxFrame
{
	public:

		MainFrame(const wxString& title);

		//----------------------------------------------------
                //      PROJECT ACTIONS
                //----------------------------------------------------
                void onNewProject(wxCommandEvent& evt);
                void onOpenProject(wxCommandEvent& evt);

		void applyThemeToAll(Theme theme);

		//----------------------------------------------------
                //      HELPER: returns currently selected ProjectPanel
                //----------------------------------------------------
		ProjectPanel* getCurrentProjectPanel();

	private:

		//----------------------------------------------------
		//	CLASS ATTRIBUTES
		//----------------------------------------------------

		//declare a wxAUI object for use across the class:
		wxAuiManager m_mgr;

		//declare a notebook for project tabs
		wxAuiNotebook* m_notebook;

		//declare the sidebar
		Sidebar* sidebar = nullptr;

		//Toolbar pointer (new) = stores a pointer to the toolbar so u can access it later 
		Toolbar* toolbar = nullptr;

		//current app theme
		Theme m_theme;

};
