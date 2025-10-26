#include "ProjectFrame.h"
#include <wx/wx.h>

#include "HandshakeDialog.h"
#include "SerialComm.h"


ProjectFrame::ProjectFrame(wxWindow* parent, const wxString& title)
	: wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300, 200))
{

	//Each Project should have an instance of the SerialComm Class:
	serialComm = std::make_unique<SerialComm>();


	//Create the main display and panel:
	wxPanel* panel = new wxPanel(this);
	
	//make a button that opens the dialogue for handshake
	wxButton* open_handshake =  new wxButton
		(
		 panel,
		 wxID_ANY,
		 "Create New Project",
		 wxPoint(20, 20),
		 wxSize(200,50)
		);

	//bind button press to an event handler that can call the handshake
	open_handshake->Bind(wxEVT_BUTTON, &ProjectFrame::onHandshake, this);

}

void ProjectFrame::onHandshake(wxCommandEvent& evt)
{
	wxLogStatus("Initiating handshake dialogue:");

	//TODO
	//now instantiate the thing that does the handshaking
	//
	//Pass a raw pointer to the SerialComm object here, to avoid transferring ownership
	HandshakeDialog* handshaker = new HandshakeDialog(this, "Handshaker", serialComm.get());
	handshaker->ShowModal();
}
