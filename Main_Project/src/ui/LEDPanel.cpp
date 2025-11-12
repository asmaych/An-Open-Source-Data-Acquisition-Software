#include "LEDPanel.h"
#include <wx/wx.h>

LEDPanel::LEDPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	SetMinSize(wxSize(50,50));
	Bind(wxEVT_PAINT, &LEDPanel::onPaint, this);
}

void LEDPanel::setState(bool state)
{
	m_state = state;
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
