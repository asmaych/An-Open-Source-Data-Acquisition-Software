#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <memory>
#include <vector>
#include "data/Run.h"
#include "sensor/Sensor.h"

class Run;
/* LiveDataWIndow class displays live data during acquisition.
   Each run is shown in its OWN tab.
   Format: time : v1, v2, v3 ...
   wxTextCntrl is used to display values in a multi-line form.
*/

class LiveDataWindow : public wxPanel
{
	public:
		/*Constructor
			- parent is the parent window (the Mainframe).
			- explicit is used to protect the compiler from passing a pointer by accident and convert it
			into a LiveDataWindow without meaning to(do not create this object automatically).
		*/
		LiveDataWindow(wxWindow* parent);
		
		//called when Start pressed
		void startNewRun(std::shared_ptr<Run> run, const std::vector<std::unique_ptr<Sensor>>& sensors);

		//called when Stop is pressed
		void stopRun();

		//called for every incoming frame
		void addFrame(double time, const std::vector<double>& values);

		//clear all liveWIndow
		void clearAll();

	private:
		wxNotebook* m_notebook = nullptr;; //notebook to create tabs
		wxGrid* m_activeGrid = nullptr; //grid in current tab to display real-time values
		std::shared_ptr<Run> m_activeRun; //currently active/open run
		std::vector<std::string> m_sensorNames; //Store sensor names for grid header
};
