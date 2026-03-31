#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <vector>
#include <string>
#include "db/DatabaseManager.h"

/*
	HardwareConfirmDialog is a class that gets called in every time a project is loaded. It displays the sensors that were saved 
	with the project and their pin assignments, and lets the user edit pins inline before confirming.
	If a pin is changed:
      		- project_sensors is updated in the DB
      		- project_calibrations pin is migrated so calibration follows the sensor to its new physical pin
      		- m_sensors in memory is updated so the rest of the app sees the new pin immediately without needing a reload

    	For now, pin values are validated to be unique in the project and between 1 and 19
*/

class HardwareConfirmDialog : public wxDialog
{
	public:
		//sensors is a vector of (name, pin) pairs loaded from the DB
        	//projectId is needed to update project_sensors and project_calibrations when pins change
        	HardwareConfirmDialog(wxWindow* parent, std::vector<std::pair<std::string,int>>& sensors, int projectId, DatabaseManager* db);

    	private:
        	//grid with one row per sensor where col 0 is for the name (read only), and col 1 is for the pin (editable by the user)
        	wxGrid* m_grid = nullptr;

        	//reference to the sensors vector so we can write back the updated pins directly
        	std::vector<std::pair<std::string,int>>& m_sensors;

        	int m_projectId;
        	DatabaseManager* m_db;

        	//called when the user clicks Confirm and validates pins and writes changes back to DB and m_sensors
        	void onConfirm(wxCommandEvent& evt);
};
