#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include <string>
#include "AddSensorDialog.h"
#include <iostream>

AddSensorDialog::AddSensorDialog(
				wxWindow* parent,
				const wxString& title,
				SensorManager* sensorManager)
	: wxDialog(
			parent,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(400,300),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
			),
	m_sensorManager(sensorManager)
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

	//verify that the name is within the character size limit
	if (name.length() > 20)
	{
		wxMessageBox("Name must be 20 characters or less!",
				"Name too long",
				wxICON_ERROR | wxOK);
		return;
	}

	//get the pin and make sure it is in the accepted range
	long pinValue = 0;
	if (!pin_field->GetValue().ToLong(&pinValue) || pinValue < 1 || pinValue > 20)
	{
		wxMessageBox("Pin must be an integer value between 1 and 20",
				"Invalid Input",
				wxICON_ERROR);
		return;
	}

	//validate both parameters with the sensorManager, 
	//sending an appropriate error message if either is already in use:
	if (m_sensorManager->nameExists(name))
	{
		wxMessageBox("This name is already in use for another sensor",
				"Invalid name",
				wxICON_ERROR | wxOK);
		return;
	}
	else if (m_sensorManager->pinExists(pinValue))
	{
		wxMessageBox("This pin is already being used for another sensor",
				"Invalid pin",
				wxICON_ERROR | wxOK);
		return;
	}

	//now instantiate the temp sensor
	std::unique_ptr<Sensor> sensor = std::make_unique<Sensor>(name, static_cast<int>(pinValue));

	//one last layer of checking
	if (m_sensorManager->addSensor(std::move(sensor)))
	{
		std::cout << "we exit the dialog\n";
		//exit the dialog
		EndModal(wxID_OK);
	}
	//otherwise, something has gone wrong, and we don't know what
	else 
	{
		wxMessageBox("Something has gone wrong, and this sensor cannot be added",
				"Unknown error",
				wxICON_ERROR | wxOK);
		return;
	}
}
