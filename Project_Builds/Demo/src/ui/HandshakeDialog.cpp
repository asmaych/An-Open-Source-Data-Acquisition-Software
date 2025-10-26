#include "HandshakeDialog.h"
#include <memory>
#include <wx/wx.h>
#include <wx/combobox.h>
#include <libserialport.h>
#include "SerialComm.h"
#include <vector>
#include <unordered_map>
#include <format>


HandshakeDialog::HandshakeDialog(wxWindow* parent, const wxString& title, SerialComm* serialComm)
    : 	wxDialog(
		    parent, 
		    wxID_ANY, 
		    title,
		    wxDefaultPosition, 
		    wxSize(500, 300),
		    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
		)
{

    	//Create the main sizer to display things in:
    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    	//pass the raw pointer parameter into our local pointer
    	this->serialComm = serialComm;

	//get the actual list of ports:
	struct sp_port ** ports = serialComm->getPortList();

    	//now create a selection and populate it with the ports
    	portChoice = new wxComboBox(this, wxID_ANY);
    	for (int i=0; ports[i] != nullptr; i++)
    	{
		//store the port description for use in a string
		std::string port_desc = sp_get_port_description(ports[i]);

		//add the port description to the drop-down
    		portChoice->Append(port_desc);

		//also add an entry to to hashmap that links the
		//description with the pointer to the port object
		portMap[port_desc] = ports[i];
    	}

	mainSizer->Add(portChoice, 0, wxEXPAND | wxALL, 10);
	
    	//now make a button to confirm the user's choice, and commence the handshake:
    	wxButton* execute_handshake = new wxButton(this, wxID_ANY, "Connect");
	mainSizer->Add(execute_handshake, 0, wxALL | wxALIGN_CENTER_HORIZONTAL,10);


	//make a status bar to show the status of the handshake
	status = new wxStaticText(this, wxID_ANY, "Select a port to begin");
	mainSizer->Add(status, 0, wxALL | wxALIGN_CENTER_HORIZONTAL,10);

	//run the macro for the sizer with all added controls
    	SetSizer(mainSizer);

    	execute_handshake->Bind(wxEVT_BUTTON, &HandshakeDialog::onPortChosen, this);
}

void HandshakeDialog::onPortChosen(wxCommandEvent& event)
{
	//first, make sure that the user actually selected something:
	int selection = portChoice->GetSelection();
	if (selection == wxNOT_FOUND)
	{
		wxLogMessage("No option selected!\nPlease select a port from the drop-down and try again.");
		return;
	}

	//store the name (not description) of the port selected:
	std::string portname = 
		sp_get_port_name(portMap[std::string(portChoice->GetStringSelection().mb_str())]);

	//This line takes the string selected by the user in the drop-down,
	//and uses it as the key in the class attribute hashmap 
	status->SetLabel(wxString::Format("Initiating handshake with port: %s, (%s)",
			portChoice->GetStringSelection(),
			portname)
			);

	//adjust the layout
	GetSizer()->Layout();
	Fit();

	//NOW WE MAKE THE HANDSHAKE
	try
	{

		//make the call to SerialComm::Handshake
		serialComm->handshake(portname);
			
		//if the execution gets here, then the handshake was successful,
		//so we write an informative message to the status bar
		status->SetLabel("Handshake successful!");
	}
	catch(const std::exception& e)
	{
		wxLogError("Error: %s", e.what());
	}
	catch(...)
	{
		wxLogError("An unkown Error occurred.");
	}
}
