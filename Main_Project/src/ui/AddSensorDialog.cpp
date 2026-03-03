#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include <string>
#include "AddSensorDialog.h"
#include <iostream>

AddSensorDialog::AddSensorDialog(
				wxWindow* parent,
				const wxString& title,
				SensorManager* sensorManager,
				SensorDatabase* sensorDatabase)
	: wxDialog(
			parent,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(400,300),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
			),
	m_sensorManager(sensorManager),
	m_sensorDatabase(sensorDatabase)
{

	//-----------------------------------------------------------------
	//ADD CONTROLS
	//-----------------------------------------------------------------
	
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//NAME FIELD:
	mainSizer->Add(new wxStaticText(this, wxID_ANY, "Sensor name:"), 0, wxALL, 5);
	name_field = new wxTextCtrl(this, wxID_ANY);
	mainSizer->Add(name_field, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	//PIN FIELD:
	mainSizer->Add(new wxStaticText(this, wxID_ANY, "Sensor pin:"), 0, wxALL, 5);
	pin_field = new wxTextCtrl(this, wxID_ANY);
	mainSizer->Add(pin_field, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	// Create the checkbox
	m_saveToDB = new wxCheckBox(this, wxID_ANY, "Save this sensor to database");
	mainSizer -> Add(m_saveToDB, 0, wxALL, 5);

	//ADD BUTTON:
	wxButton* add_button = new wxButton(this, wxID_ANY, "Add", wxDefaultPosition, wxDefaultSize);
	mainSizer->Add(add_button, 0, wxALIGN_CENTER | wxALL, 10);
	
	//BIND BUTTON TO EVENT HANDLER:
	add_button->Bind(wxEVT_BUTTON, &AddSensorDialog::onAddPressed,this);

	//set up all the elements on the dialog:
	SetSizerAndFit(mainSizer);
}

void AddSensorDialog::onAddPressed(wxCommandEvent& evt)
{
	//make a temp sensor for now to validate
	
	//get the name from the box and convert to std::string
	std::string name = name_field->GetValue().ToStdString();

	//get and validate the integer pin from pin_field
	long pinValue = 0;
	if (!pin_field->GetValue().ToLong(&pinValue) || pinValue < 1 || pinValue > 20)
	{
		wxMessageBox("Pin must be an integer value between 1 and 20",
				"Invalid Input",
				wxICON_ERROR);
		return;
	}

	//now instantiate the temp sensor
	std::unique_ptr<Sensor> sensor = std::make_unique<Sensor>(name, static_cast<int>(pinValue));

	//now validate
	if (m_sensorManager->addSensor(std::move(sensor)))
	{
		std::cout << "Sensor added to project successfully\n";

		//save to db if user wants
		if(m_saveToDB && m_saveToDB -> IsChecked()){
			m_sensorDatabase -> saveSensors(name);
			std::cout << "Sensor saved to db: " << name << "\n";
		}
		//exit the dialog
		EndModal(wxID_OK);
	}
	else
	{
		//otherwise, prompt the user to enter unique values
		wxMessageBox("Either name or pin specified is already in use, please choose another",
				"Invalid Sensor Configuration",
				wxICON_ERROR | wxOK);
		return;
	}
}
