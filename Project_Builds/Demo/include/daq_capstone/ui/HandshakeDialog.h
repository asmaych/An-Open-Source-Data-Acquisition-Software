#pragma once
#include <memory>	//include for smart pointers
#include <wx/wx.h>
#include "SerialComm.h"

class HandshakeDialog : public wxDialog
{
public:
    HandshakeDialog(wxWindow* parent, const wxString& title, SerialComm* serialComm);


private:
    SerialComm* serialComm;
    void onPortChosen(wxCommandEvent& event);

};
