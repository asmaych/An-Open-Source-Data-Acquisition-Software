#pragma once
#include <wx/wx.h>
#include "MainFrame.h"

class ProjectPanel;

class ProjectConfigDialog : public wxDialog
{
    public:
    ProjectConfigDialog(wxWindow* parent, ProjectPanel* project, MainFrame* mainFrame);

    void onChangeReadStrategy(wxCommandEvent &event);

private:
    ProjectPanel* m_project = nullptr;

    //control for theme
    wxButton* m_theme_button = nullptr;

    //text control field for entering sensor sample rate
    wxTextCtrl* sample_rate_field;

    //button for setting and updating sensor sample rate
    wxButton* sample_rate_confirm;


    //selection box for choosing which sensor getter method to use (raw,voltage,mapped)
    wxComboBox* getterSelection;

    //button for confirming changes to getter method selection
    wxButton* confirm_getter;

    bool m_darkmode = false;

    void onNewSampleRate(wxCommandEvent &evt);

    void onToggleTheme(wxCommandEvent& evt);

    MainFrame* m_mainFrame;

    SensorManager * m_sensorManager = nullptr;


};
