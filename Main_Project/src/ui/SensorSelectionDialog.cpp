#include "SensorSelectionDialog.h"

SensorSelectionDialog::SensorSelectionDialog(wxWindow* parent, const std::vector<std::string>& sensorNames)
		      : wxDialog(parent, wxID_ANY, "Select Sensors", wxDefaultPosition, wxSize(300,400))
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//convert std::vector<std::string> to wxArrayString
	wxArrayString choices;
	for(const auto& name : sensorNames){
		choices.Add(name);
	}

	//checklist box with all sensors
	m_checkList = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	//add the checklist bow to the sizer
	mainSizer -> Add(m_checkList, 1, wxEXPAND | wxALL, 10);

	// OK / Cancel buttons
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

	//for safety we check if the checklist box exists
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
