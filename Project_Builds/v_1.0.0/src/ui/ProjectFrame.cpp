#include "ProjectFrame.h"
#include <wx/wx.h>

#include "HandshakeFrame.h"
#include "SerialComm.h"


ProjectFrame::ProjectFrame(wxWindow* parent, const wxString& title)
	: wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300, 200))
{


	//Create the main display and panel:
	wxPanel* panel = new wxPanel(this);
	
	//make a button that opens the dialogue for handshake
	wxButton* open_shake =  new wxButton
		(
		 panel,
		 wxID_ANY,
		 "Create New Project",
		 wxPoint(20, 20),
		 wxSize(200,50)
		);

	//bind button press to an event handler that can call the handshake
	open_shake->Bind(wxEVT_BUTTON, &ProjectFrame::onHandshake, this);

}

void ProjectFrame::onHandshake(wxCommandEvent& evt)
{
	wxLogStatus("Calling handshaker:");

	//instantiate an instance of the SerialComm class for use in the handshake
	serialComm = new SerialComm();

	//TODO
	//now instantiate the thing that does the handshaking
	HandshakeFrame* handshaker = new HandshakeFrame(this, "Handshaker", *serialComm);
	handshaker->Show(true);
}
