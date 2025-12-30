#pragma once
#include <wx/wx.h>
#include "data/DataSession.h"
#include "ui/Events.h"
#include "sensor/Sensor.h"
#include <vector>

/* LiveDataWIndow class shows live values from sensor.
   It is a simple window that updates every time a new value is received from the sensor.
   I am usong wxTextCntrl to display values in a multi-line form.
   It also stores a reference to the DataSession object, so every new value is saved to the session as well as shown live.
*/

class LiveDataWindow : public wxFrame
{
	public:
		/*constructor where parent is the parent window (the Mainframe) and session is a pointer to DataSession
		here i am using explicit cause i don't want someone to accidentally pass two pointers and have the compiler
		convert them into a LiveDataWindow without meaning to(do not create this object automatically).
		*/
		LiveDataWindow(wxWindow* parent, const std::vector<Sensor*>& activeSensors);
		
		//Called by ProjectPanel when new values arrive
		void addValue(double value);

		// clears the display (used when user presses reset)
		void clearDisplay();

	private:
		void onFlush(wxTimerEvent& evt);

		std::vector<double> m_buffer;
		wxTimer m_timer; //Gui update timer (each 100 ms for now)
		wxTextCtrl* m_display; //Multi-line text box to display real-time values
		
		std::vector<Sensor*> m_sensors; //dynamically tracks sensors
};
