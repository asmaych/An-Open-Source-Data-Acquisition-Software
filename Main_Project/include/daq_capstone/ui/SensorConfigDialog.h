#pragma once

#include <vector>
#include "SerialComm.h"
#include "SensorManager.h"
#include <wx/listctrl.h>
#include <wx/wx.h>
#include "controllers/SessionController.h"

class SensorConfigDialog : public wxDialog
{
	public:
		SensorConfigDialog(wxWindow* parent,
				const wxString& title,
			       	SerialComm* serialComm,
			       	SensorManager* sensorManager,
				std::vector<std::unique_ptr<Sensor>>& sensors);


	private:
		//-------------------------------------------------------------------------------------------
		//CLASS ATTRIBUTES
		//-------------------------------------------------------------------------------------------

		//a raw pointer will be passed to this dialog so that the
		//SerialComm object for the related project can be used to
		//send commands to the connected microcontroller
		SerialComm* m_serialComm;

		//a raw pointer to the vector of sensors is passed into this
		//dialog, so that this dialog can add or remove sensors from
		//the vector
		SensorManager* m_sensorManager;

		//button for adding new sensors
		wxButton* add_sensor;

		//button for removing a selected sensor
		wxButton* remove_sensor;

		//button for calibrating a selected sensor
		wxButton* calibrate_sensor;

		//table for displaying added sensors
		wxListCtrl* m_list;

		//reference to a vector of sensors
		std::vector<std::unique_ptr<Sensor>>& m_sensors;

		//-------------------------------------------------------------------------------------------
		//HELPER FUNCTIONS
		//-------------------------------------------------------------------------------------------

		//this is used to populate the table with the contents of the sensor vector
		void populateTable();

		//-------------------------------------------------------------------------------------------
		//EVENT HANDLERS
		//-------------------------------------------------------------------------------------------

		//this is used to launch the AddSensorDialog when a user clicks the add_sensor button
		void onAddSensorPressed(wxCommandEvent& evt);

		//this is used to remove the selected sensor when a user clicks the remove_sensor button
		void onRemoveSensorPressed(wxCommandEvent& evt);

		//this is used to add the selected sensor to the project when a user clicks addToProject button
		//void onAddToProject(wxCommandEvent& evt);

		void onRowDblClick(wxMouseEvent& evt);

		//this is used to hand the event that a user selects a sensor to calibrate
		void onCalibratePressed(wxCommandEvent& evt);

		void onNewSampleRate(wxCommandEvent &evt);
};
