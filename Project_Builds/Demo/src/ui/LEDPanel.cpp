#include "LEDPanel.h"
#include <wx/wx.h>

LEDPanel::LEDPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), m_state(false)
{
	Bind(wxEVT_PAINT, &LEDPanel::onPaint, this);
}

void LEDPanel::setState(bool on)
{
	m_state = on;
	Refresh();
}

void LEDPanel::onPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	dc.SetBrush(wxBrush(m_state ? *wxGREEN : *wxRED));
	dc.SetPen(*wxBLACK_PEN);

	wxSize sz = GetSize();
	dc.DrawCircle(sz.GetWidth()/2, sz.GetHeight()/2, sz.GetWidth()/2-1);
}
