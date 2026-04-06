#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>
#include <string>
#include <wx/aui/auibook.h>
#include "db/DatabaseManager.h"

/*
    OpenProjectDialog is a class that replaces wxSingleChoiceDialog for opening projects. It has some features as:
    	- Real-time search filtering
      	- Double-click to open
      	- Three dots menu (⋮) → Edit DB mode
      	- Edit DB mode: checklist with Delete Data Only/Delete Everything/Cancel
*/
class OpenProjectDialog : public wxDialog
{
    	public:
        	OpenProjectDialog(wxWindow* parent, DatabaseManager* db, const std::vector<std::string>& openProjects, wxAuiNotebook* notebook);

        	//returns the selected project name, empty if none selected
        	wxString getSelectedProject() const;

    	private:
        	DatabaseManager* m_db = nullptr;

        	//all project names loaded from DB
        	std::vector<std::string> m_allProjects;

		std::vector<std::string> m_openProjects;

		wxAuiNotebook* m_notebook = nullptr;

        	//currently displayed (filtered) project names
        	std::vector<std::string> m_filteredProjects;

        	wxTextCtrl*  m_search  = nullptr;
        	wxListBox*   m_list    = nullptr;
       	 	wxButton*    m_menuBtn = nullptr; // the ⋮ button

        	wxString m_selectedProject;

        	//rebuilds m_list from m_filteredProjects
        	void rebuildList();

        	//filters m_allProjects by search text into m_filteredProjects, real time filtering
        	void onSearch(wxCommandEvent& evt);

        	//double click on a project → select and close
        	void onDoubleClick(wxCommandEvent& evt);

        	// ⋮ button → show popup menu
        	void onMenuButton(wxCommandEvent& evt);

        	//opens the edit DB dialog
        	void openEditDialog();
};
