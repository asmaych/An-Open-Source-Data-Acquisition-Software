#include "HandshakeDialog.h"
#include <memory>
#include <wx/wx.h>
#include <wx/combobox.h>
#include <libserialport.h>
#include "SerialComm.h"
#include <vector>
#include <unordered_map>
#include <format>
#include "Events.h"


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

	//this is to hold the drop-down menu of ports, and the button used to refresh it
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	//pass the raw pointer parameter into our local pointer
	this->serialComm = serialComm;

	//now create a selection and populate it with the ports
	portChoice = new wxComboBox(this, wxID_ANY);

	//initialize the port list
	loadPorts();

	//add the drop-down menu to the top sizer
	topSizer->Add(portChoice, 3, wxEXPAND | wxALL, 10);

	//make the button to refresh the ports
	wxButton* refresh_ports = new wxButton(this, wxID_ANY,"Refresh");
	//add it to the top sizer
	topSizer->Add(refresh_ports, 1, wxEXPAND | wxALL, 10);
	//bind the event handler for the button
	refresh_ports->Bind(wxEVT_BUTTON, &HandshakeDialog::onRefreshPorts, this);

	//add the top sizer to the main one.
	mainSizer->Add(topSizer, 0, wxEXPAND | wxALL, 10);
	
	//now make a button to confirm the user's choice, and commence the handshake:
	wxButton* execute_handshake = new wxButton(this, wxID_ANY, "Connect");
	mainSizer->Add(execute_handshake, 0, wxALL | wxALIGN_CENTER_HORIZONTAL,10);
	//bind the event handler for the button
	execute_handshake->Bind(wxEVT_BUTTON, &HandshakeDialog::onPortChosen, this);


	//make a status bar to show the status of the handshake
	status = new wxStaticText(this, wxID_ANY, "Select a port to begin");
	mainSizer->Add(status, 0, wxALL | wxALIGN_CENTER_HORIZONTAL,10);

	//run the macro for the sizer with all added controls
	SetSizer(mainSizer);

}

void HandshakeDialog::loadPorts() {
	//first, clear out the old contents of the combo box
	portChoice->Clear();

	//also clear out the map for port descriptions to names
	m_portMap.clear();

	//next, re-populate the combobox
	serialComm->scanPorts();
	struct sp_port** ports = serialComm->getPortList();
	for (int i=0; ports[i] != nullptr; i++)
	{
		//store the port description for use in a string
		const char* desc = sp_get_port_description(ports[i]);
		std::string port_desc = desc;

		//add the port description to the drop-down
		portChoice->Append(port_desc);

		//also add an entry to the hashmap that links the
		//description with the pointer to the port object
		m_portMap[port_desc] = sp_get_port_name(ports[i]);
	}
}

void HandshakeDialog::onRefreshPorts(wxCommandEvent& event) {
	loadPorts();
}

void HandshakeDialog::onPortChosen(wxCommandEvent& event)
{
	if (!handshakecompleted)
	{
	//first, make sure that the user actually selected something:
	int selection = portChoice->GetSelection();
	if (selection == wxNOT_FOUND)
	{
		wxLogMessage("No option selected!\nPlease select a port from the drop-down and try again.");
		return;
	}

	//store the name (not description) of the port selected:
	std::string desc = portChoice -> GetStringSelection().ToStdString();
	std::string portname = m_portMap[desc];

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
		if (serialComm->handshake(portname))
		{
			
			//if the execution gets here, then the handshake was successful,
			//so we write an informative message to the status bar
			status->SetLabel("Handshake successful!");
			handshakecompleted = true;


			//now set the trigger for handshake success
			wxThreadEvent evt(wxEVT_HANDSHAKE);
			evt.SetPayload(true);
			wxQueueEvent(this->GetParent(), evt.Clone());
		}
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
	else
	{
		wxLogStatus("Handshake already successful!");
	}
}
