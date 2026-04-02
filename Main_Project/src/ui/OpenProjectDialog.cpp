#include "OpenProjectDialog.h"
#include <wx/menu.h>
#include <algorithm>
#include <wx/checklst.h>

OpenProjectDialog::OpenProjectDialog(wxWindow* parent, DatabaseManager* db)
    		  : wxDialog(parent, wxID_ANY, "Open Project", wxDefaultPosition, wxSize(400, 450), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      		    m_db(db)
{
    	//load all projects from DB
    	m_db -> loadProjects(m_allProjects);
    	m_filteredProjects = m_allProjects;

    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    	//Top bar: search + ⋮
    	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    	m_search = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxTE_PROCESS_ENTER);
    	m_search -> SetHint("Search projects...");
    	m_search -> Bind(wxEVT_TEXT, &OpenProjectDialog::onSearch, this);

    	m_menuBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("⋮"), wxDefaultPosition, wxSize(40, -1));
    	m_menuBtn -> Bind(wxEVT_BUTTON, &OpenProjectDialog::onMenuButton, this);

    	topSizer -> Add(m_search,  1, wxEXPAND | wxRIGHT, 5);
    	topSizer -> Add(m_menuBtn, 0);
    	mainSizer -> Add(topSizer, 0, wxEXPAND | wxALL, 10);

    	//project list
    	m_list = new wxListBox(this, wxID_ANY);
    	m_list -> Bind(wxEVT_LISTBOX_DCLICK, &OpenProjectDialog::onDoubleClick, this);
    	mainSizer -> Add(m_list, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	//Hint
    	wxStaticText* hint = new wxStaticText(this, wxID_ANY, "Double-click a project to open it.");
    	hint -> SetForegroundColour(wxColour(120, 120, 120));
    	mainSizer -> Add(hint, 0, wxALL, 10);

    	SetSizer(mainSizer);
    	rebuildList();
    	Centre();
}


wxString OpenProjectDialog::getSelectedProject() const
{
    	return m_selectedProject;
}


void OpenProjectDialog::rebuildList()
{
    	m_list -> Clear();
    	for(auto& name : m_filteredProjects)
        	m_list -> Append(name);
}


void OpenProjectDialog::onSearch(wxCommandEvent& evt)
{
    	wxString query = m_search -> GetValue().Lower();
    	m_filteredProjects.clear();

	std::cout << "searching projects: query = '" << query << "' allProjects = " << m_allProjects.size();

	for(auto& name : m_allProjects){
        	wxString wx = wxString(name).Lower();
        	if(query.IsEmpty() || wx.Contains(query))
            		m_filteredProjects.push_back(name);
    	}

	std::cout << " filtered = " << m_filteredProjects.size() << "\n";

    	rebuildList();
}


void OpenProjectDialog::onDoubleClick(wxCommandEvent& evt)
{
    	int sel = m_list -> GetSelection();
    	if(sel == wxNOT_FOUND)
        	return;

    	m_selectedProject = m_list -> GetString(sel);
    	EndModal(wxID_OK);
}


void OpenProjectDialog::onMenuButton(wxCommandEvent& evt)
{
    	//show a small popup menu below the ⋮ button
    	wxMenu menu;
    	menu.Append(1, "Edit Database");

    	int choice = GetPopupMenuSelectionFromUser(menu, m_menuBtn -> GetPosition() + wxPoint(0, m_menuBtn -> GetSize().GetHeight()));

    	if(choice == 1)
        	openEditDialog();
}


void OpenProjectDialog::openEditDialog()
{
    	//reload fresh project list for the edit dialog
    	std::vector<std::string> projects;
    	m_db -> loadProjects(projects);

    	if(projects.empty()){
        	wxMessageBox("No projects in database.", "Edit Database");
        	return;
    	}

    	//build the edit dialog
    	wxDialog editDlg(this, wxID_ANY, "Edit Project Database", wxDefaultPosition, wxSize(420, 480), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    	//search bar inside edit dialog
    	wxTextCtrl* editSearch = new wxTextCtrl(&editDlg, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    	editSearch -> SetHint("Search projects...");
    	sizer -> Add(editSearch, 0, wxEXPAND | wxALL, 10);

    	//checklist of projects
    	wxArrayString choices;
    	for(auto& p : projects)
        	choices.Add(p);

    	wxCheckListBox* checkList = new wxCheckListBox(&editDlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
    	sizer -> Add(checkList, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	//wire up search to filter the checklist
    	editSearch -> Bind(wxEVT_TEXT, [&](wxCommandEvent&){ wxString query = editSearch -> GetValue().Lower(); checkList -> Clear();
        						     for(auto& p : projects){
            							wxString wx = wxString(p).Lower();
            							if(query.IsEmpty() || wx.Contains(query))
                							checkList -> Append(p);
        						      }
    							    });

    	//info label
    	wxStaticText* info = new wxStaticText(&editDlg, wxID_ANY, "Select projects to delete, then choose an action.");
    	info -> SetForegroundColour(wxColour(120, 120, 120));
    	sizer -> Add(info, 0, wxALL, 10);

    	//three buttons at the bottom
    	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

    	wxButton* dataOnlyBtn = new wxButton(&editDlg, wxID_ANY, "Delete Data Only");
    	wxButton* everythingBtn = new wxButton(&editDlg, wxID_ANY, "Delete Everything");
    	wxButton* cancelBtn = new wxButton(&editDlg, wxID_CANCEL, "Cancel");

    	//delete Data Only — keeps project setup and calibrations but removes runs, frames, collect points, ui_state
    	dataOnlyBtn -> Bind(wxEVT_BUTTON, [&](wxCommandEvent&){ std::vector<std::string> selected;
       	 							for(unsigned int i = 0; i < checkList->GetCount(); ++i)
            								if(checkList -> IsChecked(i))
                								selected.push_back(checkList -> GetString(i).ToStdString());

								if(selected.empty()){
            								wxMessageBox("No projects selected.", "Delete Data Only");
            								return;
        							}

        							int confirm = wxMessageBox(wxString::Format(
											"Delete run data for %zu project(s)?\n" 
											"Sensor setup and calibrations will be kept.",
                				    					selected.size()), "Confirm Delete Data", 
											wxYES_NO | wxICON_WARNING);

								if(confirm != wxYES) 
									return;

       								for(auto& name : selected){
            								int id = m_db -> getProjectID(name);
            								if(id >= 0)
										m_db -> deleteProjectData(id);
        							}

        							wxMessageBox("Run data deleted successfully.", "Done");
        							editDlg.EndModal(wxID_OK);
    								});

    	//delete Everything — removes project, sensors (local only), all data, calibrations, and setup
    	everythingBtn -> Bind(wxEVT_BUTTON, [&](wxCommandEvent&){ std::vector<std::string> selected;
        							for(unsigned int i = 0; i < checkList->GetCount(); ++i)
            								if(checkList -> IsChecked(i))
                								selected.push_back(checkList -> GetString(i).ToStdString());

        							if(selected.empty()){
            								wxMessageBox("No projects selected.", "Delete Everything");
            								return;
        							}

        							int confirm = wxMessageBox(wxString::Format(
                							"Permanently delete %zu project(s) and ALL their data?\n"
                							"This cannot be undone.", selected.size()),
									"Confirm Delete Everything", wxYES_NO | wxICON_ERROR);

        							if(confirm != wxYES)
									return;

        							for(auto& name : selected){
            								int id = m_db -> getProjectID(name);
            								if(id >= 0)
										m_db -> deleteProject(id);
        							}

        							wxMessageBox("Projects deleted successfully.", "Done");
        							editDlg.EndModal(wxID_OK);
    								});

	btnSizer -> Add(dataOnlyBtn, 0, wxRIGHT, 5);
    	btnSizer -> Add(everythingBtn, 0, wxRIGHT, 5);
    	btnSizer -> AddStretchSpacer();
    	btnSizer -> Add(cancelBtn,     0);

    	sizer -> Add(btnSizer, 0, wxEXPAND | wxALL, 10);
    	editDlg.SetSizer(sizer);
    	editDlg.Centre();

    	//always reload after edit dialog closes regardless how it was closed
	if(editDlg.ShowModal() == wxID_OK){
	    	m_db -> loadProjects(m_allProjects);
    		m_filteredProjects = m_allProjects;
    		rebuildList();
	}

}
