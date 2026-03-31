#include "HardwareConfirmDialog.h"
#include <wx/grid.h>
#include <set>

HardwareConfirmDialog::HardwareConfirmDialog(wxWindow* parent, std::vector<std::pair<std::string,int>>& sensors, int projectId,
					     DatabaseManager* db)

		      : wxDialog(parent, wxID_ANY, "Hardware Setup", wxDefaultPosition, wxSize(380, 300), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      			m_sensors(sensors),
      			m_projectId(projectId),
      			m_db(db)
{
    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    	//informative message at the top so the user knows what to do
    	wxStaticText* msg = new wxStaticText(this, wxID_ANY, "Do you want to keep the same hardware setup (sensor pins)?\n");
    	mainSizer -> Add(msg, 0, wxALL, 10);

    	//for the grid, we are doing one row per sensor, two columns (Name | Pin), name column is read-only, meaning
	//the user can only change pins
    	m_grid = new wxGrid(this, wxID_ANY);
    	m_grid -> CreateGrid(static_cast<int>(sensors.size()), 2);
    	m_grid -> SetColLabelValue(0, "Sensor");
    	m_grid -> SetColLabelValue(1, "Pin");
    	m_grid -> SetRowLabelSize(0); //hide row numbers (not useful = no need)

    	//column widths
    	m_grid -> SetColSize(0, 200);
    	m_grid -> SetColSize(1, 80);

    	//populate rows
    	for(size_t i = 0; i < sensors.size(); ++i){
        	int row = static_cast<int>(i);

        	//sensor name is  read only, and just for reference
        	m_grid -> SetCellValue(row, 0, sensors[i].first);
        	m_grid -> SetReadOnly(row, 0, true);

        	//pin is editable, pre-filled with the saved value
        	m_grid -> SetCellValue(row, 1, wxString::Format("%d", sensors[i].second));

        	//use integer editor for the pin column so the user gets immediate feedback if they type something invalid
        	m_grid->SetCellEditor(row, 1, new wxGridCellNumberEditor(1, 19));
    	}

    	m_grid -> EnableEditing(true);
    	m_grid -> SetMinSize(wxSize(-1, 150));
    	mainSizer -> Add(m_grid, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

    	//buttons
    	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    	wxButton* confirmBtn = new wxButton(this, wxID_ANY, "Confirm");
    	wxButton* cancelBtn  = new wxButton(this, wxID_CANCEL, "Cancel");

    	btnSizer -> AddStretchSpacer();
    	btnSizer -> Add(confirmBtn, 0, wxRIGHT, 5);
    	btnSizer -> Add(cancelBtn,  0);

	mainSizer -> Add(btnSizer, 0, wxEXPAND | wxALL, 10);

    	SetSizer(mainSizer);
    	Layout();
    	Centre();

    	confirmBtn -> Bind(wxEVT_BUTTON, &HardwareConfirmDialog::onConfirm, this);
}


void HardwareConfirmDialog::onConfirm(wxCommandEvent& evt)
{
    	//force any in-progress cell edit to commit before we read values
    	m_grid -> SaveEditControlValue();
    	m_grid -> DisableCellEditControl();

    	//first, we read and validate all pins
    	std::vector<int> newPins;
    	std::set<int> seen;

    	for(size_t i = 0; i < m_sensors.size(); ++i){
        	int row = static_cast<int>(i);
        	wxString val = m_grid->GetCellValue(row, 1).Trim();

        	long pin = 0;
        	if(!val.ToLong(&pin) || pin < 1 || pin > 19){
            		wxMessageBox(wxString::Format("Invalid pin for sensor '%s'.\n" "Pins must be integers between 1 and 19.", m_sensors[i].first), "Invalid Pin", wxOK | wxICON_ERROR);
            		return;
        	}

        	if(seen.count(static_cast<int>(pin))){
            		wxMessageBox(wxString::Format("Pin %ld is assigned to more than one sensor.\n" "Each sensor must have a unique pin.", pin), "Duplicate Pin", wxOK | wxICON_ERROR);
            		return;
        	}

        	seen.insert(static_cast<int>(pin));
        	newPins.push_back(static_cast<int>(pin));
    	}

    	//second, we apply the changes
    	for(size_t i = 0; i < m_sensors.size(); ++i){
        	int oldPin = m_sensors[i].second;
        	int newPin = newPins[i];

        	//skip sensors whose pin didn't change
        	if(oldPin == newPin)
            		continue;

        	int sensorId = m_db -> getSensorID(m_sensors[i].first);
        	if(sensorId < 0)
            		continue;

        	std::cout << "Pin change: '" << m_sensors[i].first << "' " << oldPin << " -> " << newPin << "\n";

        	//update project_sensors so the new pin is persisted. We delete the old row and insert a fresh one because the pin
		//column is part of the UNIQUE constraint, an UPDATE would conflict if another sensor already held the new pin (which 
		//we already validated against above)
        	m_db -> updateProjectSensorPin(m_projectId, sensorId, oldPin, newPin);

       	 	//migrate project_calibrations to the new pin so the calibration follows the sensor to its new physical position
       	        m_db -> migrateCalibrationPin(m_projectId, sensorId, oldPin, newPin);

        	//update the in-memory sensor so the rest of the app (SensorConfigDialog, SessionController...) sees the new pin 
		//immediately without needing a full reload
        	m_sensors[i].second = newPin;
    	}

    	EndModal(wxID_OK);
}
