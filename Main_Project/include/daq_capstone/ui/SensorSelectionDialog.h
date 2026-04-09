#pragma once
#include <wx/wx.h>
#include <wx/checklst.h>
#include <vector>
#include <string>
#include "db/DatabaseManager.h"
#include "CalibrationPoint.h"

/**
 * @brief GUI object that allows user to select active sensors
 *
 * This is a dialog that lets the user select sensors from the global catalogue to load into a project, it has features as:
 *     	- Real-time search filtering as the user types
 *     	- Calibration preview next to each sensor name so the user can tell sensors apart without loading them individually
 *     	- Three dots menu (⋮) top right → Edit Database mode which let the user remove sensors from the catalogue
 *     	- Multi-select via checkboxes, check as many as needed 
*/
class SensorSelectionDialog : public wxDialog
{
	public:
		//the constructor takes the parent window and a list of sensor names
		SensorSelectionDialog(wxWindow* parent, const std::vector<std::string>& sensorNames, DatabaseManager* db);

		//get indexes of selected sensors after user presses OK, it correctly maps filtered positions back to original indexes
        	//so selection is correct even when the list is filtered
		std::vector<int> getSelectedIndexes() const;

	private:
		//checklist showing sensor names + calibration previews
		wxCheckListBox* m_checkList = nullptr;

		//pointer to db manager
		DatabaseManager* m_db = nullptr;

		//search bar for real-time filtering
        	wxTextCtrl* m_search = nullptr;

        	//Three dots button (⋮) that opens the popup menu
        	wxButton* m_menuBtn = nullptr;

        	//full list of sensor names as loaded from DB never modified so we can always restore after filtering
        	std::vector<std::string> m_allSensors;

        	//currently displayed sensors after search filter is applied
        	std::vector<std::string> m_filteredSensors;

		//builds a short preview string from the first two calibration points, return no calibration if none exist
		wxString buildCalibrationPreview(const std::string& sensorName);

		//rebuilds the checklist from m_filteredSensors called on construction and every time the search changes
        	void rebuildList();

        	//called on every keyenter in the search bar, filters m_allSensors into m_filteredSensors and rebuilds the list
        	void onSearch(wxCommandEvent& evt);

        	//called when the ⋮ button is clicked and shows a small popup menu with "Edit Database" option
        	void onMenuButton(wxCommandEvent& evt);

        	//opens the edit database dialog where the user can search and remove sensors from the global catalogue
        	void openEditDialog();
};
