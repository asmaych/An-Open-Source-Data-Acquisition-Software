#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <vector>
#include "CalibrationPoint.h"

/*
    	CalibrationCompareDialog is a class that allows us to show two calibration tables side by side, the current globally
	saved calibration on the left and the new one the user just entered on the right.

	Rows where the raw or mapped values differ are highlighted in yellow so the user can spot changes instantly before deciding 
	to overwrite.

    	Both grids are read-only. The user either confirms the overwrite or cancels, leaving the global calibration untouched.
*/

class CalibrationCompareDialog : public wxDialog
{
    	public:
        	CalibrationCompareDialog(wxWindow* parent, const std::vector<CalibrationPoint>& current, const std::vector<CalibrationPoint>& incoming);

    	private:
        	wxGrid* m_currentGrid  = nullptr;
        	wxGrid* m_incomingGrid = nullptr;

        	//populates a grid with calibration points, again read only
        	void populateGrid(wxGrid* grid, const std::vector<CalibrationPoint>& points);

        	//highlights rows that differ between the two tables, for now it uses exact double comparison, any diff triggers highlight
        	void highlightDifferences(const std::vector<CalibrationPoint>& current, const std::vector<CalibrationPoint>& incoming);
};
