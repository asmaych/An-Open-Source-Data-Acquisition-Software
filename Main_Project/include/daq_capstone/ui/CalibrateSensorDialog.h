#pragma once

#include <wx/wx.h>
#include "SensorManager.h"
#include "Sensor.h"
#include "string"
#include "CalibrationTableDialog.h"

/**
 * @brief GUI object used to allow user to select which Calibrator implementation to configure for a selected Sensor
 *
 * When this GUI dialog is created, it displays a list of options to the user.
 * Each option represents a calibration method that the user can choose.
 * The calibration method chosen is applied to the sensor which is selected by the user prior to dialog launch.
 */
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

