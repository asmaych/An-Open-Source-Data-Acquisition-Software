#pragma once
#include <memory>	//include for smart pointers
#include <wx/wx.h>
#include "SerialComm.h"
#include <libserialport.h>
#include <unordered_map>

class HandshakeDialog : public wxDialog
{
public:
    HandshakeDialog(wxWindow* parent, const wxString& title, SerialComm* serialComm);


private:

    //this will be a raw pointer passed through the constructor
    SerialComm* serialComm;

    //this will be used to map a port selection with an actual port object
    std::unordered_map<std::string, struct sp_port*> portMap;

    //declaring the dropdown menu so it can be used across the class
    wxComboBox* portChoice;

    //declare a status textbox so that we can send updates to it from across
    //the class
    wxStaticText* status;

    void onPortChosen(wxCommandEvent& event);

};
