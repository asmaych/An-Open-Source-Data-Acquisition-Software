#pragma once
#include <wx/wx.h>
#include <wx/checklst.h>
#include <vector>
#include <string>

/**
 * @brief GUI object that allows user to select active sensors
 *
 * This is a dialog window that allows the user to select which sensors to collect from in a Run.
 * It displays a checklist of all sensors in the project.
 * It returns the selected sensor indexes.
*/
class SensorSelectionDialog : public wxDialog
{
	public:
		//the constructor takes the parent window and a list of sensor names
		SensorSelectionDialog(wxWindow* parent, const std::vector<std::string>& sensorNames);

		//get indexes of selected sensors after user presses OK
		std::vector<int> getSelectedIndexes() const;

	private:
		//wxwidgets checklist box
		wxCheckListBox* m_checkList;

		//stores selected indexes
		std::vector<int> m_selectedIndexes;
};
