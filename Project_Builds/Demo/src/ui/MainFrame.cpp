#include "MainFrame.h"
#include "ProjectFrame.h"
#include <wx/wx.h>


MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title)
{
	/* This is a panel that must always be present. Due to unique behaviour of
	 * the wxwidgets library, this is required to avoid the situation that 
	 * other wx objects like buttons or menus don't scale unpredictably.
	 *
	 * By first adding this panel, and then adding everything to the panel,
	 * all later controls behave predictably.
	 */
	wxPanel* panel = new wxPanel(this);

	wxButton* button = new wxButton(panel, wxID_ANY, "New Project", wxPoint(300, 275), wxSize(200,50));
	button->Bind(wxEVT_BUTTON, &MainFrame::onCreateProject, this);

	CreateStatusBar();


}

void MainFrame::onCreateProject(wxCommandEvent& evt)
{
	//Display an informative message to users
	wxLogStatus("Creating new project");



	//Now create a new window for the project
	ProjectFrame* project = new ProjectFrame(this, "New project");
	project->Show();
}

