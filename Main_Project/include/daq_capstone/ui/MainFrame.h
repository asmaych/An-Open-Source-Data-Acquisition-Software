#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "Sidebar.h"
#include "ProjectPanel.h"
#include "HandshakeDialog.h"
#include "Theme.h"
#include "Toolbar.h"
#include "MainFrame.h"

class ProjectPanel;
class Sidebar;
class Toolbar;

/**
 *	@brief Main GUI page used to launch into projects
 *
 * MainFrame class is the main application window. It contains:
 *
 *	-	A toolbar with features like Start, Stop, Collect, Sensor and Graph,
 *		that are disabled unless a project is open, and where actions like start/
 *		stop/collect/graph won't run unless a sensor is selected.
 *
 *	-	A sidebar on the left with New Project, Load Project, Connect, Sensors, and other user options.
 *
 *	-	A central Display window for showing ProjectPanel instances.
 *
 *	Each ProjectPanel instance is wrapped into a wxAUINotebook with switchable tabs.
*/
class MainFrame : public wxFrame
{
	public:

		MainFrame(const wxString& title);

		void toggleTheme(wxCommandEvent& evt);

		void toggleTheme();

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

		Theme getTheme();

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
