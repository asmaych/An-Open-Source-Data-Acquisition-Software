#pragma once

#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include "db/DatabaseManager.h"
#include <string>
#include "ProjectPanel.h"
class AddSensorDialog : public wxDialog
{
	public:
		AddSensorDialog(
				wxWindow* parent,
				const wxString& title,
				SensorManager* sensorManager,
				DatabaseManager* Database,
				int projectID
				);
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
		DatabaseManager* m_Database;

		//id for the project this dialog belongs to
		int m_projectID; 

		//------------------------------------------------------
		//EVENT HANDLERS
		//------------------------------------------------------

		//for when the "add" button is pressed
		void onAddPressed(wxCommandEvent& evt);



};
