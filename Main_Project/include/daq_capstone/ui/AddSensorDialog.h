#pragma once

#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include "sensor/SensorDatabase.h"
#include <string>

/**
 * @brief GUI object used to allow user-entry of new Sensor parameters.
 *
 * This class is used to provide a GUI to the user that allows them to manually enter details related to the new sensor
 * they wish to configure. Additionally, the user is able to select an option that saves the sensor to the global
 * database of sensors for easier reuse.
 */
class AddSensorDialog : public wxDialog
{
	public:
		AddSensorDialog(
				wxWindow* parent,
				const wxString& title,
				SensorManager* sensorManager,
				SensorDatabase* sensorDatabase);
	private:
		//------------------------------------------------------
		//CLASS MEMBERS
		//------------------------------------------------------

		//this will be pass through the constructor as a 
		//raw pointer that originates in parent of parent
		//ProjectPanel object. This will be used to validate
		//new sensor object parameters
		SensorManager* m_sensorManager;

		//text box to input a name for the sensor
		wxTextCtrl* name_field;

		//text box to input an integer for the sensor pin
		wxTextCtrl* pin_field;

		//checkbox for saving a sensor in the db
		wxCheckBox* m_saveToDB;

		//pointer to db
		SensorDatabase* m_sensorDatabase;

		//------------------------------------------------------
		//EVENT HANDLERS
		//------------------------------------------------------

		//for when the "add" button is pressed
		void onAddPressed(wxCommandEvent& evt);



};
