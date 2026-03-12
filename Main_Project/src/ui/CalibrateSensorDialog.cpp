#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include <string>
#include "CalibrateSensorDialog.h"
#include <iostream>
#include "Interpolator.h"

CalibrateSensorDialog::CalibrateSensorDialog(
						wxWindow* parent,
						const wxString& title,
						SensorManager* sensorManager,
						const long sensor_index)
	: wxDialog(
			parent,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(400,300),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
		  ),
	m_sensorManager(sensorManager),
	m_sensor_index(sensor_index)
{
	
	//-----------------------------------------------------------------
	//ADD CONTROLS
	//-----------------------------------------------------------------
	
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//INFORMATIVE MESSAGE:
	mainSizer->Add(new wxStaticText(this, wxID_ANY, "Select a calibration method:"), 0, wxALL, 5);

	//TABLE ENTRY METHOD:
	wxButton* table_entry = new wxButton(this, wxID_ANY, "Manual Table");
	table_entry->SetToolTip("This is for manually entering datapoints in a table");
	mainSizer->Add(table_entry, 0, wxALIGN_CENTER | wxALL, 10);
	//BIND THE BUTTON:
	table_entry->Bind(wxEVT_BUTTON, &CalibrateSensorDialog::onManualTablePressed, this);

	//DIRECT EQUATION ENTRY
	wxButton* equation_entry = new wxButton(this, wxID_ANY, "Equation");
	equation_entry->SetToolTip("This is for users who have an equation for voltage -> values");
	mainSizer->Add(equation_entry, 0, wxALIGN_CENTER | wxALL, 10);
	//BIND THE BUTTON:
	equation_entry->Bind(wxEVT_BUTTON, &CalibrateSensorDialog::onEquationPressed, this);

	//IMAGE-BASED CALIBRATION FROM DATASHEET GRAPH:
	wxButton* datasheet_entry = new wxButton(this, wxID_ANY, "Datasheet graph");
	datasheet_entry->SetToolTip("This is for when the user has a calibration curve graph on a datasheet");
	mainSizer->Add(datasheet_entry, 0, wxALIGN_CENTER | wxALL, 10);
	//BIND THE BUTTON:
	equation_entry->Bind(wxEVT_BUTTON, &CalibrateSensorDialog::onDatasheetPressed, this);


	//adjust the fitting of all the controls:
	SetSizerAndFit(mainSizer);
}

void CalibrateSensorDialog::onManualTablePressed(wxCommandEvent& evt)
{
	//TODO implement logic
	//call the table-based calibration dialog
	CalibrationTableDialog calibration_table(this,m_sensorManager, m_sensor_index);
	if (calibration_table.ShowModal() == wxID_OK)
	{
		EndModal(wxID_OK);
	}
	
}

void CalibrateSensorDialog::onEquationPressed(wxCommandEvent& evt)
{
	//TODO implement logic
}

void CalibrateSensorDialog::onDatasheetPressed(wxCommandEvent& evt)
{
	//TODO implement logic
}
