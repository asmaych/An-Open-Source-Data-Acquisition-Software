#pragma once
#include <wx/wx.h>
#include "data/DataSession.h"

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
		explicit LiveDataWindow(wxWindow* parent, DataSession* session);
		
		//default constructor (we do not need custom cleanup)
		~LiveDataWindow() override = default;

		//append the buffer of data received in serialComm to LiveDisplay
		void appendBuffer(const std::string& buffer);

		//Add a value to both the live display and the DataSession
		void addValue(double value);

		//Close handler - we keep default behavior (destruction)
		void clearDisplay();

	private:
		DataSession* m_session; //pointer to sensor's DataSession
		wxTextCtrl* m_display; //Multi-line text box to display real-time values
		
		

};
