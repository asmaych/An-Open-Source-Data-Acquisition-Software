#include "LEDPanel.h"
#include <wx/wx.h>

/* LED Panel is a simple indicator panel for live sensor status. it draws a colored circle (green/red) based on boolean input
   It can be used in ProjectPanel to indicate sensor readings, but for now it is used there as a status indicator.
*/

//constructor
LEDPanel::LEDPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	// set minimum size for display
	SetMinSize(wxSize(50,50));
	Bind(wxEVT_PAINT, &LEDPanel::onPaint, this);
}

void LEDPanel::setState(bool state)
{
	//update LED state and refresh our GUI
	m_state = state;
	Refresh();
}

//Draw LED
void LEDPanel::onPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	//Green if on, red if off
	dc.SetBrush(wxBrush(m_state ? *wxGREEN : *wxRED));
	dc.SetPen(*wxBLACK_PEN);

	wxSize sz = GetSize();
	//draw a circle filling panel
	dc.DrawCircle(sz.GetWidth()/2, sz.GetHeight()/2, sz.GetWidth()/2-1);
}
