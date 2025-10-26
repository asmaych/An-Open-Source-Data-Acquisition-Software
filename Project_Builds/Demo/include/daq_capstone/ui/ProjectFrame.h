#pragma once

//this include is for the use of smart-pointers
#include <memory>

#include <wx/wx.h>
#include "SerialComm.h"

class ProjectFrame : public wxFrame
{
	public:
		ProjectFrame(wxWindow* parent, const wxString& title);


	private:
		void onHandshake(wxCommandEvent& evt);

		std::unique_ptr<SerialComm> serialComm;

};

