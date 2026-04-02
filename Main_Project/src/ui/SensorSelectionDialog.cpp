#include "SensorSelectionDialog.h"
#include <wx/listctrl.h>

SensorSelectionDialog::SensorSelectionDialog(wxWindow* parent, const std::vector<std::string>& sensorNames, DatabaseManager* db)
		      : wxDialog(parent, wxID_ANY, "Select Sensors", wxDefaultPosition, wxSize(550,450)), m_db(db),
      			m_allSensors(sensorNames), m_filteredSensors(sensorNames)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//Top bar: search field + ⋮ menu butt
	//The search bar filters sensors in real time as the user types
    	//The ⋮ button opens a popup menu for database management
    	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

   	m_search = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    	m_search -> SetHint("Search sensors...");
    	m_search -> Bind(wxEVT_TEXT, &SensorSelectionDialog::onSearch, this);

    	//three dots button, opens popup menu on click
    	m_menuBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("⋮"), wxDefaultPosition, wxSize(40, -1));
    	m_menuBtn -> Bind(wxEVT_BUTTON, &SensorSelectionDialog::onMenuButton, this);

    	topSizer -> Add(m_search, 1, wxEXPAND | wxRIGHT, 5);
    	topSizer -> Add(m_menuBtn, 0);
    	mainSizer -> Add(topSizer, 0, wxEXPAND | wxALL, 10);

    	//Column headers are sensor name on the left and calibration preview on the right
    	wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    	headerSizer -> Add(new wxStaticText(this, wxID_ANY, "Sensor Name"), 1, wxLEFT, 10);
    	headerSizer -> Add(new wxStaticText(this, wxID_ANY, "Calibration Preview"), 2, wxLEFT, 10);
    	mainSizer -> Add(headerSizer, 0, wxEXPAND | wxBOTTOM, 4);

    	//Checklist
    	//a monospace font is used so the name and calibration columns line up cleanly across all rows.
    	m_checkList = new wxCheckListBox(this, wxID_ANY);
    	m_checkList -> SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    	mainSizer -> Add(m_checkList, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	// OK / Cancel buttons
    	wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    	btnSizer -> AddButton(new wxButton(this, wxID_OK));
    	btnSizer -> AddButton(new wxButton(this, wxID_CANCEL));
    	btnSizer -> Realize();
    	mainSizer -> Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 10);

    	SetSizer(mainSizer);

    	//populate the checklist for the first time with all sensors
    	rebuildList();
}


void SensorSelectionDialog::rebuildList()
{
    	//clear existing items before repopulating
    	m_checkList -> Clear();

    	//add one row per filtered sensor with its calibration preview
    	for(auto& name : m_filteredSensors){
        	//only attempt calibration preview if DB is available, but if m_db is null just show no calibration without crashing
        	wxString preview = m_db ? buildCalibrationPreview(name) : wxString("No calibration");

        	//format: "sensor_name  |  raw->mapped, ..."
        	//monospace font makes this line up cleanly
        	wxString label = wxString::Format("%-20s  |  %s", name, preview);
        	m_checkList -> Append(label);
    	}
}


void SensorSelectionDialog::onSearch(wxCommandEvent& evt)
{
    	//get the current search text and convert to lowercase for case insensitive matching (trim not considered here)
    	wxString query = m_search -> GetValue().Lower();

    	//rebuild filtered list, keep only sensors whose name contains the query string, meaning empty query shows all sensors
    	m_filteredSensors.clear();
    	for(auto& name : m_allSensors){
        	wxString wx = wxString::FromUTF8(name).Lower();
        	if(query.IsEmpty() || wx.Contains(query))
            		m_filteredSensors.push_back(name);
    	}

    	//repopulate the checklist with the filtered results
    	rebuildList();
}


void SensorSelectionDialog::onMenuButton(wxCommandEvent& evt)
{
    	//show a small popup menu directly below the ⋮ buttoncurrently contains only "Edit Database" but can be extended
    	wxMenu menu;
    	menu.Append(1, "Edit Database");

    	int choice = GetPopupMenuSelectionFromUser(menu, m_menuBtn -> GetPosition() + wxPoint(0, m_menuBtn -> GetSize().GetHeight()));

    	if(choice == 1)
        	openEditDialog();
}


void SensorSelectionDialog::openEditDialog()
{
    	//reload the full catalogue from DB to ensure we show the latest state including any changes made in a previous edit session
    	std::vector<std::string> catalogueSensors;
    	if(m_db)
		m_db -> loadSensorTemplates(catalogueSensors);

    	if(catalogueSensors.empty()){
        	wxMessageBox("No sensors in the catalogue.", "Edit Database");
        	return;
    	}

    	//build the edit dialog, separate window on top of the main dialog
    	wxDialog editDlg(this, wxID_ANY, "Edit Sensor Database", wxDefaultPosition, wxSize(550, 480), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    	//Search bar inside edit dialog that filters the catalogue list in real time, same as the main dialog
    	wxTextCtrl* editSearch = new wxTextCtrl(&editDlg, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    	editSearch -> SetHint("Search sensors...");
    	sizer -> Add(editSearch, 0, wxEXPAND | wxALL, 10);

    	//Column headers
    	wxBoxSizer* hdrSizer = new wxBoxSizer(wxHORIZONTAL);
    	hdrSizer -> Add(new wxStaticText(&editDlg, wxID_ANY, "Sensor Name"), 1, wxLEFT, 10);
    	hdrSizer -> Add(new wxStaticText(&editDlg, wxID_ANY, "Calibration Preview"), 2, wxLEFT, 10);
    	sizer -> Add(hdrSizer, 0, wxEXPAND | wxBOTTOM, 4);

    	//Checklist
    	wxCheckListBox* checkList = new wxCheckListBox(&editDlg, wxID_ANY);
    	checkList -> SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    	//helper to rebuild the edit checklist with an optional search filter that is defined here so both the initial populate
    	//and the search binding can use the same logic
    	auto rebuildEditList = [&](const wxString& query){checkList -> Clear();
        			   for(auto& name : catalogueSensors){
            				//apply filter if query is non-empty
            				wxString wx = wxString::FromUTF8(name).Lower();
            				if(!query.IsEmpty() && !wx.Contains(query))
                				continue;

            				wxString preview = m_db ? buildCalibrationPreview(name) : wxString("No calibration");

            				checkList -> Append(wxString::Format("%-20s  |  %s", name, preview));
        			   }
    	};

    	//populate the checklist for the first time with all catalogue sensors
    	rebuildEditList("");
    	sizer -> Add(checkList, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	//bind search to filter the checklist in real time
    	editSearch -> Bind(wxEVT_TEXT, [&](wxCommandEvent&){rebuildEditList(editSearch -> GetValue().Lower());});

    	//info label, which explains what removing a sensor from the catalogue means (it stays in any projects that already use it)
    	wxStaticText* info = new wxStaticText(&editDlg, wxID_ANY, "Select sensors to remove from the catalogue.\n"
        							  "They will remain in any projects that use them.");
    	info -> SetForegroundColour(wxColour(120, 120, 120));
    	sizer -> Add(info, 0, wxALL, 10);

    	//Buttons
    	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    	wxButton* removeBtn = new wxButton(&editDlg, wxID_ANY, "Remove Selected");
    	wxButton* cancelBtn = new wxButton(&editDlg, wxID_CANCEL, "Cancel");

    	//bind remove button which sets user_saved=0 for each selected sensor which removes it from the catalogue without deleting 
	//the row
    	removeBtn -> Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
        							//collect names of checked sensors
        							std::vector<std::string> selected;
        							for(unsigned int i = 0; i < checkList->GetCount(); ++i)
            								if(checkList -> IsChecked(i))
                								selected.push_back(catalogueSensors[i]);

        							if(selected.empty()){
            								wxMessageBox("No sensors selected.", "Remove Selected");
            								return;
        							}

        							//ask the user to confirm before removing
        							int confirm = wxMessageBox(wxString::Format(
                							"Remove %zu sensor(s) from the catalogue?\n"
                							"They will stay in any projects that use them.",
                							selected.size()), "Confirm Remove",
            								wxYES_NO | wxICON_WARNING);

        							if(confirm != wxYES)
									return;

        							//unlist each selected sensor meaning set user_saved = 0
        							for(auto& name : selected){
									int id = m_db -> getSensorID(name);
            								if(id >= 0)
										m_db -> unlistSensor(id);
        							}

        							wxMessageBox("Sensors removed from catalogue.", "Done");
        							editDlg.EndModal(wxID_OK);
    	});

    	btnSizer -> Add(removeBtn, 0, wxRIGHT, 5);
    	btnSizer -> AddStretchSpacer();
    	btnSizer -> Add(cancelBtn, 0);

    	sizer -> Add(btnSizer, 0, wxEXPAND | wxALL, 10);
    	editDlg.SetSizer(sizer);
    	editDlg.Centre();

    	//reload the main sensor list after edit dialog closes so any removals are reflected immediately without reopening the dialog
   	if(editDlg.ShowModal() == wxID_OK){
        	if(m_db)
			m_db -> loadSensorTemplates(m_allSensors);
        	m_filteredSensors = m_allSensors;
        	rebuildList();
    	}
}


//getSelectedIndexes will return a vector of integers representing the positions of checked items
std::vector<int> SensorSelectionDialog::getSelectedIndexes() const
{
	std::vector<int> selected; //vector to store selected indexes

	//first  we check if the checklist box exists
	if(!m_checkList){
		return selected;
	}

	//loop through all items in the checklist
	for(unsigned int i = 0; i < m_checkList -> GetCount(); ++i){

		if(!m_checkList -> IsChecked(i)){
			continue;
		}

		//get the actual sensor name from m_filteredSensors, no string parsing needed
		std::string filteredName = m_filteredSensors[i];

		//extract sensor name from the label by taking the part before | and stripping whitespace
        	wxString label = m_checkList -> GetString(i);
        	wxString name  = label.BeforeFirst('|').Trim();

       	 	//find this sensor's original index in m_allSensors
        	for(size_t j = 0; j < m_allSensors.size(); ++j){

            		if(wxString::FromUTF8(m_allSensors[j]).Trim() == name){
                		selected.push_back(static_cast<int>(j));
                		break;
			} 
		}
	}

	//return the vector containing indexes of all checked snesors
	return selected;
}



//builds a short preview string from the first two calibration points and returns no calibration if none exist
wxString SensorSelectionDialog::buildCalibrationPreview(const std::string& sensorName)
{
    	if(!m_db)
        	return "No calibration";

    	int sensorId = m_db -> getSensorID(sensorName);
    	if(sensorId < 0)
        	return "No calibration";

    	std::string type;
    	std::vector<CalibrationPoint> points;

    	if(!m_db -> loadGlobalCalibration(sensorId, type, points) || points.empty())
        	return "No calibration";

    	//show first two points as a fingerprint of this calibration
    	wxString preview;
    	size_t count = std::min(points.size(), size_t(2));
    	for(size_t i = 0; i < count; ++i){
        	if(i > 0) 
			preview += ",  ";
        	preview += wxString::Format("%.2f->%.2f", points[i].raw, points[i].mapped);
    	}

    	if(points.size() > 2)
        	preview += ", ...";

    	return preview;
}
