#include "OpenProjectDialog.h"
#include "ProjectPanel.h"
#include <wx/menu.h>
#include <algorithm>
#include <wx/checklst.h>
#include <wx/aui/auibook.h>

/**
 * @brief builds the dialog with a search bar, a 3-column project list (Name | Runs | Created), and a ⋮ menu button that gives access 
 *        to edit DB and sort options.
 */
OpenProjectDialog::OpenProjectDialog(wxWindow* parent, DatabaseManager* db, const std::vector<std::string>& openProjects, wxAuiNotebook* notebook)
    		  : wxDialog(parent, wxID_ANY, "Open Project", wxDefaultPosition, wxSize(400, 450), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      		    m_db(db),
		    m_openProjects(openProjects),
		    m_notebook(notebook)
{
    	//load all projects from DB
    	m_db -> loadProjectsWithInfo(m_allInfos);

	//populate m_allProjects string list for openEditDialog
	for(auto& info : m_allInfos)
        	m_allProjects.push_back(info.name);

	//start with no filter applied
	m_filteredInfos = m_allInfos;
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

    	//project list (wxLC_REPORT = table/grid view)
	//three cols: name(left), runs(right aligned count), created(date)
    	m_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_NONE);
	m_list -> AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
    	m_list -> AppendColumn("Runs", wxLIST_FORMAT_RIGHT, 60);
    	m_list -> AppendColumn("Created", wxLIST_FORMAT_LEFT, 160);

	//double-click on a row selects the project and closes the dialog
    	m_list -> Bind(wxEVT_LIST_ITEM_ACTIVATED, &OpenProjectDialog::onDoubleClick, this);
    	mainSizer -> Add(m_list, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	//Hint
    	wxStaticText* hint = new wxStaticText(this, wxID_ANY, "Double-click a project to open it.");
    	hint -> SetForegroundColour(wxColour(120, 120, 120));
    	mainSizer -> Add(hint, 0, wxALL, 10);

    	SetSizer(mainSizer);
    	rebuildList();
    	Centre();
}


/**
 * @brief returns the name of the project the user selected, empty if none
 */

wxString OpenProjectDialog::getSelectedProject() const
{
    	return m_selectedProject;
}


/**
 * @brief clears and refills the wxListCtrl from m_filteredInfos, called after every search query change or sort operation
 */

void OpenProjectDialog::rebuildList()
{
    	m_list->DeleteAllItems();

	for(size_t i = 0; i < m_filteredInfos.size(); ++i){
       		auto& info = m_filteredInfos[i];

        	//insert the project name in column 0
        	long idx = m_list -> InsertItem((long)i, info.name);

        	//run count in column 1
        	m_list -> SetItem(idx, 1, wxString::Format("%d", info.runCount));

        	//creation date in column 2 (already trimmed to "YYYY-MM-DD HH:MM")
        	m_list -> SetItem(idx, 2, info.createdAt);
    	}
}


/**
 * @brief filters the project list in real time as the user types in the search box, matches against project name 
	  (case-insensitive substring)
 */

void OpenProjectDialog::onSearch(wxCommandEvent& evt)
{
    	wxString query = m_search -> GetValue().Lower();

	m_filteredInfos.clear();
	m_filteredProjects.clear();

	std::cout << "searching projects: query = '" << query << "' allProjects = " << m_allProjects.size();

	for(auto& proj : m_allInfos){
        	wxString wx = wxString(proj.name).Lower();
        	if(query.IsEmpty() || wx.Contains(query)){
			m_filteredInfos.push_back(proj);
            		m_filteredProjects.push_back(proj.name);
		}
    	}

	std::cout << " filtered = " << m_filteredProjects.size() << "\n";

    	rebuildList();
}


/**
 * @brief called when the user double-clicks a row in the list, stores the selected project name and closes the dialog with wxID_OK
 */
void OpenProjectDialog::onDoubleClick(wxListEvent& evt)
{
    	long idx = evt.GetIndex();
    	if(idx < 0 || idx >= (long)m_filteredInfos.size())
        	return;

    	m_selectedProject = m_filteredInfos[idx].name;
    	EndModal(wxID_OK);
}


/**
 * @brief called whenever the user wants to duplicate a project that already exists
 */
void OpenProjectDialog::duplicateSelected()
{
    	//get currently highlighted row
    	long idx = m_list -> GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    	if(idx < 0 || idx >= (long)m_filteredInfos.size()){
        	wxMessageBox("Please select a project to duplicate first.", "No Selection");
        	return;
    	}

    	std::string originalName = m_filteredInfos[idx].name;

    	//generate a unique name: hello → hello_2 → hello_3 ...
    	std::string newName = originalName + "_2";
    	int suffix = 2;
    	while(m_db -> getProjectID(newName) >= 0){
        	suffix++;
        	newName = originalName + "_" + std::to_string(suffix);
    	}

    	int originalId = m_db -> getProjectID(originalName);
    	if(originalId < 0){
        	wxMessageBox("Could not find original project.", "Error");
        	return;
    	}

    	//duplicate in DB
    	int newId = m_db -> duplicateProject(originalId, newName);
    	if(newId < 0){
        	wxMessageBox("Failed to duplicate project.", "Error");
        	return;
    	}

    	wxMessageBox(wxString::Format("Project duplicated as '%s'.\n" "Same sensors and calibrations, no run data.", newName), "Duplicate Created", wxOK | wxICON_INFORMATION);

    	//reload list
    	m_db -> loadProjectsWithInfo(m_allInfos);
    	m_allProjects.clear();
    	for(auto& info : m_allInfos)
        	m_allProjects.push_back(info.name);

    	wxCommandEvent dummy;
    	onSearch(dummy);
}


/**
 * @brief shows the ⋮ popup menu with three options:
 *          1. Edit Database, opens the delete/manage dialog
 *          2. Sort by newest first, re-sorts the list descending by created_at
 *          3. Sort by oldest first, re-sorts the list ascending by created_at
 */

void OpenProjectDialog::onMenuButton(wxCommandEvent& evt)
{
    	wxMenu menu;
   	menu.Append(1, "Edit Database");
    	menu.AppendSeparator();
    	menu.Append(2, "Sort: Newest first");
    	menu.Append(3, "Sort: Oldest first");
	menu.AppendSeparator();
    	menu.Append(4, "Duplicate Project");

    	int choice = GetPopupMenuSelectionFromUser(menu, m_menuBtn -> GetPosition() + wxPoint(0, m_menuBtn -> GetSize().GetHeight()));

    	if(choice == 1){
        	openEditDialog();
    	}
    	else if(choice == 2){
        	//sort descending, larger date string = more recent
        	m_sortNewest = true;

		std::sort(m_filteredInfos.begin(), m_filteredInfos.end(), [](const DatabaseManager::ProjectInfo& a, const DatabaseManager::ProjectInfo& b){ 
				return a.createdAt > b.createdAt;
				});

		//keep m_filteredProjects in sync for openEditDialog
        	m_filteredProjects.clear();
        	for(auto& info : m_filteredInfos)
            		m_filteredProjects.push_back(info.name);
 
        	rebuildList();
    	}
    	else if(choice == 3){
        	//sort ascending, smaller date string = older
        	m_sortNewest = false;
        	std::sort(m_filteredInfos.begin(), m_filteredInfos.end(), [](const DatabaseManager::ProjectInfo& a, const DatabaseManager::ProjectInfo& b){ 
				return a.createdAt < b.createdAt;
            			});
        	//keep m_filteredProjects in sync
        	m_filteredProjects.clear();
        	for(auto& info : m_filteredInfos)
            		m_filteredProjects.push_back(info.name);
 
        	rebuildList();
    	}
	else if(choice == 4)
		duplicateSelected();
}


/**
 * @brief opens a secondary dialog that lets the user select projects from a checklist and either delete their run data or delete 
 *        them entirely.
 *        "Delete Data Only": removes runs, frames, collect_points, ui_state, keeps project row, sensors, and calibrations.
 *                             Allowed even if the project is currently open cause the open panel is reset immediately.
 *        "Delete Everything": removes the project row and everything under it. Blocked if the project is currently open in a tab.
 *        The outer project list is always reloaded after this dialog closes regardless of how it was dismissed.
 */
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

        							//warn if any selected project is currently open
    								for(auto& name : selected){
        								if(std::find(m_openProjects.begin(), m_openProjects.end(), name) != m_openProjects.end()){
            									wxMessageBox(wxString::Format("'%s' is currently open.\n"
													    "Continue anyway?", name),
                											   "Project Is Open",
                											   wxYES_NO | wxICON_INFORMATION);

            									break;
        								}
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
        								//capture name and m_notebook explicitly for the inner scope
    									wxAuiNotebook* nb = m_notebook;
    									std::string projectName = name;

    									if(nb){
        									for(size_t i = 0; i < nb -> GetPageCount(); ++i){
            										if(nb -> GetPageText(i).ToStdString() == projectName){
                										auto* panel = dynamic_cast<ProjectPanel*>(nb -> GetPage(i));
                										if(panel)
                    											panel -> resetSessionData();
                										break;
            										}
        									}
    									}
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

								//block deletion of any project that is currently open
								for(auto& name : selected){
        								if(std::find(m_openProjects.begin(), m_openProjects.end(), name) != m_openProjects.end()){
            									wxMessageBox(wxString::Format("'%s' is currently open.\n"
                    									"Please close the tab before deleting it.", name),
                									"Project Is Open", wxOK | wxICON_WARNING);
            									return;
        								}
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
	editDlg.ShowModal();
    	m_db -> loadProjectsWithInfo(m_allInfos);
    	m_allProjects.clear();
    	for(auto& info : m_allInfos)
        	m_allProjects.push_back(info.name);
 
    	//reapply current search filter so the list stays consistent
    	wxCommandEvent dummy;
    	onSearch(dummy);
}
