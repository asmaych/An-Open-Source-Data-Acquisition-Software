#pragma once
#include <wx/wx.h>
#include "Theme.h"

class ProjectPanel;

class ProjectConfigDialog : public wxDialog
{
    public:
    ProjectConfigDialog(wxWindow* parent, ProjectPanel* project);

    void onNewSampleRate(wxCommandEvent &evt);

    bool dark;

private:
    ProjectPanel* m_project = nullptr;

    //control for theme
    wxCheckBox* m_theme_toggle = nullptr;

    //text control field for entering sensor sample rate
    wxTextCtrl* sample_rate_field;

    //button for setting and updating sensor sample rate
    wxButton* sample_rate_confirm;


};
