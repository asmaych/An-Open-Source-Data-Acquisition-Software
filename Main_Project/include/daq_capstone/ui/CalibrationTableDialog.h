#pragma once

#include <vector>
#include <wx/wx.h>
#include <wx/grid.h>
#include <string>
#include "CalibrationPoint.h"
#include "SensorManager.h"

/**
 * @brief GUI object used to allow user entry of raw-mapped pairs into a table for use in an Interpolator
 *
 * When this dialog is created, it loads a table of raw-mapped pairs, that the user can add to or modify.
 * These values are used to create a vector of CalibrationPoint objects, which are loaded into an Interpolator
 * for use in a specific Sensor selected earlier by the user.
 *
 * If there is no current calibration configured for the selected Sensor, the table will be blank. Otherwise,
 * the calibration table will be loaded with the current values.
 */
class CalibrationTableDialog : public wxDialog
{

	public:
		CalibrationTableDialog(
				wxWindow* parent, 
				SensorManager* sensorManager, 
				long sensor_index);

		//helper function to convert the data from the table into a vector
		void getCalibrationPoints();

		void loadCalibrationPoints(const std::vector<CalibrationPoint> &table);

		void onCellEdited(wxGridEvent &evt);

		void onEnterPressed(wxKeyEvent &evt);

	private:
		wxGrid* m_grid;

		long m_sensor_index;
		SensorManager* m_sensorManager;

		std::unique_ptr<std::vector<CalibrationPoint>> m_table;
		std::unique_ptr<Interpolator> m_interpolator;

		void onOkPressed(wxCommandEvent& evt);

		std::vector<CalibrationPoint> const * m_calibrationPoints = nullptr;
};
