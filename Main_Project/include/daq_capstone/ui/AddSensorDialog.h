#pragma once

#include <wx/wx.h>
#include "Sensor.h"
#include "SensorManager.h"
#include <string>

class AddSensorDialog : public wxDialog
{
	public:
		AddSensorDialog(wxWindow* parent,
				const wxString& title,
				SensorManager* sensorManager);
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

		//------------------------------------------------------
		//EVENT HANDLERS
		//------------------------------------------------------

		//for when the "add" button is pressed
		void onAddPressed(wxCommandEvent& evt);



};
