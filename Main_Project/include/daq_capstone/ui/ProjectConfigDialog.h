#pragma once
#include <wx/wx.h>
#include "MainFrame.h"

class ProjectPanel;

/**
 * @brief Gui object used to modify Application and Project Parameters at runtime
 *
 * This is a dialog that allows the user to switch between light and dark mode, select polling rate,
 * and choose between what type of sensor reading to capture. In the future, any additional configuration
 * requirements will be implemented on the front-end here.
 *
 */
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
