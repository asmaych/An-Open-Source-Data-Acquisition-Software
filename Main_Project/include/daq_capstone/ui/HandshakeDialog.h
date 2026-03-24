#pragma once
#include <memory>	//include for smart pointers
#include <wx/wx.h>
#include "SerialComm.h"
#include <libserialport.h>
#include <unordered_map>

/**
 * @brief GUI object used to allow the user to browse and select a microcontroller to connect to
 *
 * When this dialog is launched, it will display a drop-down menu to the user, which is populated by a scan of all
 * serial ports by SerialComm. The user can select and option, and click a button to connect to the microcontroller at
 * that port. Additionally, the user is given a refresh button, which uses SerialComm to re-scan the ports, and
 * generates an updated drop-down menu for the user.
 */
class HandshakeDialog : public wxDialog
{
public:
    HandshakeDialog(wxWindow* parent, const wxString& title, SerialComm* serialComm);

    void loadPorts();

private:

    //this will be a raw pointer passed through the constructor
    SerialComm* serialComm;

    //this will be used to map a port selection with an actual port object
    std::unordered_map<std::string, std::string> m_portMap;

    //declaring the dropdown menu so it can be used across the class
    wxComboBox* portChoice;

    //declare a status textbox so that we can send updates to it from across
    //the class
    wxStaticText* status;

    void onPortChosen(wxCommandEvent& event);

    void onRefreshPorts(wxCommandEvent &event);


    bool handshakecompleted = false;

};
