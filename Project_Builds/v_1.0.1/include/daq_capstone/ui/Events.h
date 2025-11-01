#pragma once
#include <wx/thread.h>

//this is the event that is triggered when the
//background polling thread recieves a reading.
//It is handled in the ProjectPanel class
wxDECLARE_EVENT(wxEVT_SERIAL_UPDATE, wxThreadEvent);

//this is the event that is triggered when a 
//handshake is conducted successfully. It is handled
//in the ProjectPanel class
wxDECLARE_EVENT(wxEVT_HANDSHAKE, wxThreadEvent);

//this is an event that represents the user clicking on
//the "Create New Project" button on the sidebar. It
//is handled in the MainFrame class
wxDECLARE_EVENT(wxEVT_PROJECT_NEW, wxCommandEvent);
