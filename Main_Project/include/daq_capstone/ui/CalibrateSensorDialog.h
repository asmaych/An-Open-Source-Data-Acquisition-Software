#pragma once

#include <wx/wx.h>
#include "SensorManager.h"
#include "Sensor.h"
#include "string"
#include "CalibrationTableDialog.h"

class CalibrateSensorDialog : public wxDialog
{
	public: 
		CalibrateSensorDialog(
					wxWindow* parent,
					const wxString& title,
					SensorManager* sensorManager,
					const long sensor_index); 

	private:
		//------------------------------------------------------
		//CLASS MEMBERS
		//------------------------------------------------------

		//this is passed through the constructor as a raw pointer
		//to SensorManager class instance that originates in 
		//ProjectPanel. This is used to handle the calibration in
		//each sensor.
		SensorManager* m_sensorManager;

		//this is used to hold the index of the sensor we wish
		//to assign a calibrator to.
		long m_sensor_index;


		//------------------------------------------------------
		//EVENT HANDLERS
		//------------------------------------------------------

		//for when the "Manual Table" button is pressed
		void onManualTablePressed(wxCommandEvent& evt);

		//for when the "Equation" button is pressed
		void onEquationPressed(wxCommandEvent& evt);

		//for when the "Datasheet graph" button is pressed
		void onDatasheetPressed(wxCommandEvent& evt);


};

