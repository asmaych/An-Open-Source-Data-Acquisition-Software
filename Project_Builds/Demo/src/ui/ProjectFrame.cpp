#include <wx/wx.h>

#include "HandshakeDialog.h"
#include "SerialComm.h"

#include <thread>
#include <atomic>

#include "LEDPanel.h"

#include "SerialEvents.h"

wxDEFINE_EVENT(wxEVT_SERIAL_UPDATE, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_HANDSHAKE, wxThreadEvent);

#include "ProjectFrame.h"

#include <mutex>


ProjectFrame::ProjectFrame(wxWindow* parent, const wxString& title)
	: wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(300, 200))
{

	//Each Project should have an instance of the SerialComm Class:
	serialComm = std::make_unique<SerialComm>();

	//Create the main display and panel:
	wxPanel* panel = new wxPanel(this);

	//make a sizer for the controls
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//make the status LED display panel
	ledIndicator = new LEDPanel(panel);

	
	//make a button that opens the dialogue for handshake
	wxButton* open_handshake =  new wxButton
		(
		 panel,
		 wxID_ANY,
		 "Create New Project",
		 wxPoint(20, 20),
		 wxSize(200,50)
		);

	mainSizer->Add(open_handshake, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
	mainSizer->Add(ledIndicator, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

	//bind button press to an event handler that can call the handshake
	open_handshake->Bind(wxEVT_BUTTON, &ProjectFrame::onHandshake, this);

	//bind the custom event to its handler
	Bind(wxEVT_SERIAL_UPDATE, &ProjectFrame::onSerialUpdate, this);
	Bind(wxEVT_HANDSHAKE, &ProjectFrame::onHandshakeSuccess, this);

	panel->SetSizerAndFit(mainSizer);

}

ProjectFrame::~ProjectFrame()
{
	/* This destructor is explicitly implemented so that the threads 
	 * created during runtime get shut down when the ProjectFrame
	 * lifetime ends
	 */
	stopBackgroundPolling();
}

void ProjectFrame::onHandshake(wxCommandEvent& evt)
{
	wxLogStatus("Initiating handshake dialogue:");

	//TODO
	//now instantiate the thing that does the handshaking
	//
	//Pass a raw pointer to the SerialComm object here, to avoid transferring ownership
	HandshakeDialog* handshaker = new HandshakeDialog(this, "Handshaker", serialComm.get());
	handshaker->Show();
}

void ProjectFrame::onSerialUpdate(wxThreadEvent& evt)
{
	std::cout <<"we get here\n";
	int reading = evt.GetInt();

	if (reading == 1)
	{
		ledIndicator->setState(true);
	}
	else if (reading == 0)
	{
		ledIndicator->setState(false);
	}
	else
		wxLogStatus("Reading updated: %d",reading);
}

void ProjectFrame::onHandshakeSuccess(wxThreadEvent& evt)
{
	bool success = evt.GetPayload<bool>();
	if (success)
	{
		//prompt the arduino to move to the next phase
		std::lock_guard<std::mutex> lock(serialMutex);
		serialComm->writeData("Begin\n");

		std::cout << "\n\n" << "test" << "\n\n";
		startBackgroundPolling();
	}
}

void ProjectFrame::startBackgroundPolling()
{
	//set the class variable boolean to true
	running = true;

	ioThread = std::thread([this]()
		{
			while (running)
			{
				std::cout << "thread iteration: we call SerialComm::writeData()\n";

				std::lock_guard<std::mutex> lock(serialMutex);
				serialComm->flush();

				int reading = serialComm->getReading();

				serialComm->flush();

				//now trip the event and run the update:
				wxThreadEvent evt(wxEVT_SERIAL_UPDATE);
				evt.SetInt(reading);
				wxQueueEvent(this, evt.Clone());

				std::cout << "right before the thread timer\n";

				//add a little pause
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				std::cout << "After the thread timer\n";
			}
		});
}

void ProjectFrame::stopBackgroundPolling()
{
	running = false;

	if (ioThread.joinable())
	{
		ioThread.join();
	}
}
