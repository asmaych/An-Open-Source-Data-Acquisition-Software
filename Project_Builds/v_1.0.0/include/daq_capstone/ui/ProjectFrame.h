#pragma once
#include <wx/wx.h>
#include "SerialComm.h"

class ProjectFrame : public wxFrame
{
	public:
		ProjectFrame(wxWindow* parent, const wxString& title);


	private:
		void onHandshake(wxCommandEvent& evt);
		SerialComm* serialComm;

};

