#pragma once
#include <wx/wx.h>
#include "Theme.h"
#include "MainFrame.h"

class ProjectPanel;

class ProjectConfigDialog : public wxDialog
{
    public:
    ProjectConfigDialog(wxWindow* parent, ProjectPanel* project, MainFrame* mainFrame);

private:
    ProjectPanel* m_project = nullptr;

    //control for theme
    wxCheckBox* m_theme_toggle = nullptr;
    wxButton* m_theme_button = nullptr;

    //text control field for entering sensor sample rate
    wxTextCtrl* sample_rate_field;

    //button for setting and updating sensor sample rate
    wxButton* sample_rate_confirm;

    bool m_darkmode = false;

    void onNewSampleRate(wxCommandEvent &evt);

    void onToggleTheme(wxCommandEvent& evt);

    MainFrame* m_mainFrame;


};
