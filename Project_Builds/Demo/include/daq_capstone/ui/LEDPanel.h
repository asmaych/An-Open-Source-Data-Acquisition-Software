#pragma once

#include <wx/wx.h>

class LEDPanel : public wxPanel
{
	public:
		LEDPanel(wxWindow* parent);
		void setState(bool on);

	private:
		//class attributes
		bool m_state = false;

		//event handler
		void onPaint(wxPaintEvent& evt);
};
