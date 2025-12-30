#pragma once
#include <wx/wx.h>
#include <wx/thread.h>

/* this is the event that is triggered when the background polling thread recieves a reading.
   It is handled in the ProjectPanel class
*/
wxDECLARE_EVENT(wxEVT_SERIAL_UPDATE, wxThreadEvent);

/* this is the event that is triggered when a handshake is conducted successfully. 
*/
wxDECLARE_EVENT(wxEVT_HANDSHAKE, wxThreadEvent);

/* this is an event that represents the user clicking on the "Create New Project" button on the sidebar.
*/
wxDECLARE_EVENT(wxEVT_PROJECT_NEW, wxCommandEvent);

/* Application level toolbar commands
*/
//wxDECLARE_EVENT(wxEVT_START_EXPERIMENT, wxCommandEvent);
//wxDECLARE_EVENT(wxEVT_STOP_EXPERIMENT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COLLECT_UPDATED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_START_STOP_TOGGLE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GRAPH_SENSOR, wxCommandEvent);

