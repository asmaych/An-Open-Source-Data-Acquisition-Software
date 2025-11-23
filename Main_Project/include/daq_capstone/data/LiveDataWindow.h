#pragma once
#include <wx/wx.h>
#include "DataSession.h"

/* LiveDataWIndow class shows live values from sensor.
   It is a simple window that updates every time a new value is received from the sensor.
   I am usong wxTextCntrl to display values in a multi-line form.
   It also stores a reference to the DataSession object, so every new value is saved to the session as well as shown live.
*/

class LiveDataWindow : public wxFrame
{
	public:
		//constructor where parent is the parent window (the Mainframe) and session is a pointer to DataSession
		LiveDataWindow(wxWindow* parent, DataSession* session);
	
		//Add a value to both the live display and the DataSession
		void addValue(double value);

	private:
		DataSession* m_session; //pointer to sensor's DataSession
		wxTextCtrl* m_display; //Multi-line text box to display real-time values
};
