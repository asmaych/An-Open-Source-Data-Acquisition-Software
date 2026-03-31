#include "SensorSelectionDialog.h"
#include <wx/listctrl.h>

SensorSelectionDialog::SensorSelectionDialog(wxWindow* parent, const std::vector<std::string>& sensorNames, DatabaseManager* db)
		      : wxDialog(parent, wxID_ANY, "Select Sensors", wxDefaultPosition, wxSize(500,400)), m_db(db)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//header labels to explain the two pieces of info shown per row
    	wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    	headerSizer -> Add(new wxStaticText(this, wxID_ANY, "Sensor Name"), 1, wxLEFT, 10);
    	headerSizer -> Add(new wxStaticText(this, wxID_ANY, "Calibration Preview"), 2, wxLEFT, 10);
    	mainSizer -> Add(headerSizer, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

    	//build one label per sensor combining name + calibration preview, following the format: "temp1   |   0.00→0.00, 1024.00→25.30, ..."
	wxArrayString choices;
    	for(const auto& name : sensorNames){
		//only attempt calibration preview if DB is available, if m_db is null, just show the name with no preview
    		wxString preview = m_db ? buildCalibrationPreview(name) : wxString("No calibration");
        	wxString label = wxString::Format("%-20s  |  %s", name, preview);
        	choices.Add(label);
    	}

    	//wxCheckListBox gives us checkboxes and multi-select for free
    	m_checkList = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);

    	//use a monospace font so the columns line up cleanly
   	m_checkList->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	//add it to the sizer
	mainSizer -> Add(m_checkList, 1, wxEXPAND | wxALL, 10);

	// OK /Cancel buttons
	wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
	btnSizer -> AddButton(new wxButton(this, wxID_OK)); //ok button
	btnSizer -> AddButton(new wxButton(this, wxID_CANCEL)); //cancel button
	btnSizer -> Realize(); //lay out the buttons properly

	//Add the button sizer to the mainSizer
	mainSizer -> Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 10);

	//set the mainSizer for the dialog, so it manages layout automatically
	SetSizer(mainSizer);

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

		//if the item at index i is checked, add it to the selected vector
		if(m_checkList -> IsChecked(i)){
			selected.push_back(i);
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
