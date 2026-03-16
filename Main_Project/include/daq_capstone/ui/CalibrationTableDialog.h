#pragma once

#include <vector>
#include <wx/wx.h>
#include <wx/grid.h>
#include <string>
#include "CalibrationPoint.h"
#include "SensorManager.h"

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
