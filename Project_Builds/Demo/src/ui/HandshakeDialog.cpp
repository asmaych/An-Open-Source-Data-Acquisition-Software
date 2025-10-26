#include "HandshakeDialog.h"
#include <memory>
#include <wx/wx.h>
#include <wx/combobox.h>
#include <libserialport.h>
#include "SerialComm.h"
#include <vector>


HandshakeDialog::HandshakeDialog(wxWindow* parent, const wxString& title, SerialComm* serialComm)
    : 	wxDialog(
		    parent, 
		    wxID_ANY, 
		    title,
		    wxDefaultPosition, 
		    wxSize(400, 200),
		    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
		)
{

    	//Create the main sizer to display things in:
    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    	//pass the raw pointer parameter into our local pointer
    	this->serialComm = serialComm;

    	//get the ports and store them in a vector:
    	//std::vector<std::string> portnames = serialComm->getPortNames();

	//get the actual list of ports:
	struct sp_port ** ports = serialComm->getPortList();


    	//now create a selection and populate it with the ports
    	wxComboBox* portChoice = new wxComboBox(this, wxID_ANY);
    	for (int i=0; ports[i] != nullptr; i++)
    	{
    		portChoice->Append(sp_get_port_description(ports[i]));
    	}

	mainSizer->Add(portChoice, 0, wxEXPAND | wxALL, 10);
	
    	//now make a button to confirm the user's choice, and commence the handshake:
    	wxButton* execute_handshake = new wxButton(this, wxID_ANY, "Connect");
	mainSizer->Add(execute_handshake, 0, wxALIGN_CENTER_HORIZONTAL,10);

	//run the macro for the sizer with all added controls
    	SetSizerAndFit(mainSizer);

    	execute_handshake->Bind(wxEVT_BUTTON, &HandshakeDialog::onPortChosen, this);
}

void HandshakeDialog::onPortChosen(wxCommandEvent& event)
{
	wxLogMessage("Connecting to port: %s", event.GetString());
}
