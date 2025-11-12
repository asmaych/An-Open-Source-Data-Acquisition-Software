#pragma once

#include <wx/wx.h>

class LEDPanel : public wxPanel
{
	public:
		//parent is the panel that contains this LED		
		LEDPanel(wxWindow* parent);

		//set the LED state to (on/off) and refresh display
		void setState(bool on);

	private:
		//LED state
		bool m_state = false;

		//event handler: draw a circle with green/red
		void onPaint(wxPaintEvent& evt);
};
