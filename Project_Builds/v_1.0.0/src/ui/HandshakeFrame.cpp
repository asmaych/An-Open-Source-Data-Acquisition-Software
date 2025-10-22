#include "HandshakeFrame.h"
#include <wx/wx.h>
#include "SerialComm.h"


HandshakeFrame::HandshakeFrame(wxWindow* parent, const wxString& title, const SerialComm& serialComm)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300, 200))
{
    //Create the main display and panel:
    wxPanel* panel = new wxPanel(this);

    wxStaticText* test = new wxStaticText(panel, wxID_ANY, wxT("shaking hands!"));

}
