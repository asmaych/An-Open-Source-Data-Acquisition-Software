#pragma once

//this include is for the use of smart-pointers
#include <memory>

#include <wx/wx.h>
#include "SerialComm.h"
#include <wx/listctrl.h>
#include <thread>
#include <atomic>
#include "LEDPanel.h"
#include <wx/thread.h>
#include <mutex>
#include "sensor/Sensor.h"
#include "Events.h"


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
		wxListCtrl* sensorList = nullptr; //table to show sensors
		std::vector<std::unique_ptr<Sensor>> sensors; //Sensor objects
		bool handshakecomplete = false;
		std::mutex serialMutex;

		//event handlers
		void onAddSensor(wxCommandEvent& evt);
		void onRemoveSensor(wxCommandEvent& evt);
		void onHandshake(wxCommandEvent& evt);
		void onSerialUpdate(wxThreadEvent& evt);
		void onHandshakeSuccess(wxThreadEvent& evt);

		//helper functions
		void startBackgroundPolling();
		void stopBackgroundPolling();

};

