#pragma once

//this include is for the use of smart-pointers
#include <memory>

#include <wx/wx.h>
#include "SerialComm.h"

#include <thread>
#include <atomic>
#include "LEDPanel.h"
#include <wx/thread.h>
#include <mutex>



class ProjectPanel : public wxPanel
{
	public:
		ProjectPanel(wxWindow* parent, const wxString& title);
		~ProjectPanel();


	private:
		//class attributes
		std::unique_ptr<SerialComm> serialComm;
		std::thread ioThread;
		std::atomic<bool> running{false};
		LEDPanel* ledIndicator = nullptr;
		bool handshakecomplete = false;
		std::mutex serialMutex;

		//event handlers
		void onHandshake(wxCommandEvent& evt);
		void onSerialUpdate(wxThreadEvent& evt);
		void onHandshakeSuccess(wxThreadEvent& evt);

		//helper functions
		void startBackgroundPolling();
		void stopBackgroundPolling();


};

