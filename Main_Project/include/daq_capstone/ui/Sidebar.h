#pragma once
#include <wx/wx.h>
#include <wx/panel.h> 
#include <wx/button.h>

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

	private:
		//Buttons for the sidebar actions: NewProject, LoadProject
		wxButton* new_project_button;
		wxButton* load_project_button;

		//Event handlers for the buttons
		void OnNewProject(wxCommandEvent& evt);
		void OnLoadProject(wxCommandEvent& evt);

		MainFrame* m_parent; //pointer to the main frame for calling project functions

};
