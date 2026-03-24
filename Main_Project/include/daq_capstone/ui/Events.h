#pragma once
#include <wx/wx.h>
#include <wx/thread.h>

/**
 * this is the event that is triggered when the background polling thread receives a reading.
 * It is handled in ProjectPanel
*/
wxDECLARE_EVENT(wxEVT_SERIAL_UPDATE, wxThreadEvent);

/**
 * this is the event that is triggered when a handshake is conducted successfully.
 * It is handled in ProjectPanel
*/
wxDECLARE_EVENT(wxEVT_HANDSHAKE, wxThreadEvent);

/**
 * This is an event that represents the user clicking on the "Create New Project" button on the sidebar.
 * It is handled in MainFrame
*/
wxDECLARE_EVENT(wxEVT_PROJECT_NEW, wxCommandEvent);

/* Application level toolbar commands
*/
//wxDECLARE_EVENT(wxEVT_START_EXPERIMENT, wxCommandEvent);
//wxDECLARE_EVENT(wxEVT_STOP_EXPERIMENT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COLLECT_UPDATED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_START_STOP_TOGGLE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_GRAPH_SENSOR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_COLLECT_NOW_POINT, wxCommandEvent);

/**
 * This is an event that represents the user clicking on the theme toggle button in ProjectConfig.
 * It is handled in MainFrame
 */
wxDECLARE_EVENT(wxEVT_THEME_TOGGLE, wxCommandEvent);
