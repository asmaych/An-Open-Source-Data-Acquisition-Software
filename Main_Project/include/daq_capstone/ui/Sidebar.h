#pragma once
#include <wx/wx.h>
#include <wx/panel.h> 
#include <wx/button.h>
#include "Theme.h"

/* Forward declaration of MainFrame to allow event handling callbacks
   In other words, I am telling the compiler that a class called MainFRame exists but not defined here, because the sidebar
   always needs to communicate with MainFrame as when a button is clicked
*/
class MainFrame;

class Sidebar : public wxPanel
{
	public:
		//a pointer to the parent window (MainFrame)
		Sidebar(MainFrame* parent);

		void refreshConnection(); //connection state only
		void applyTheme(Theme theme);
	private:
		//Buttons for the sidebar actions: NewProject, LoadProject
		wxButton* new_project_button;
		wxButton* load_project_button;
		wxButton* m_connect_button;

		wxButton* m_sensor_button;

		//Event handlers for the buttons
		void OnNewProject(wxCommandEvent& evt);
		void OnLoadProject(wxCommandEvent& evt);
		void OnConnect(wxCommandEvent& evt);
		void OnAddRemoveSensor(wxCommandEvent& evt);

		MainFrame* m_parent; //pointer to the main frame for calling project functions

};
