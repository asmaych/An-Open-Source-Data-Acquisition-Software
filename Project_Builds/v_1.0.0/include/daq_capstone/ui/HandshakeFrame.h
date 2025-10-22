#pragma once
#include <wx/wx.h>
#include "SerialComm.h"

class HandshakeFrame : public wxFrame
{
public:
    HandshakeFrame(wxWindow* parent, const wxString& title, const SerialComm& serialComm);


private:
    SerialComm* serialComm;

};