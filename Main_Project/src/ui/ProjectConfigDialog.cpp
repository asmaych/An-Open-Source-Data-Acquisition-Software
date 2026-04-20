#include "ProjectConfigDialog.h"
#include "ProjectPanel.h"
#include "SensorManager.h"
#include "Theme.h"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(daq_resources);

wxBitmapBundle ProjectConfigDialog::LoadEmbeddedSVG(const std::string& path, wxSize size) {
    auto fs = cmrc::daq_resources::get_filesystem();
    auto file = fs.open(path);
    return wxBitmapBundle::FromSVG(
        reinterpret_cast<const wxByte *>(file.begin()),
        file.end() - file.begin(),
        size
    );
}

ProjectConfigDialog::ProjectConfigDialog(wxWindow* parent,
                                         ProjectPanel* project,
                                         MainFrame* mainframe)
    : wxDialog(parent,
               wxID_ANY,
               "Project Configuration",
               wxDefaultPosition,
               wxSize(400, 300),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_project(project),
    m_mainFrame(mainframe)
{
    m_darkmode = (m_mainFrame->getTheme() == Theme::Dark);

    //top-level sizer to hold all the controls and sub-sizers
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    //sizer for the sensor sample rate configuration
    wxBoxSizer* sampleSizer = new wxBoxSizer(wxHORIZONTAL);

    //create and add the setting for the application theme. Set the button icon to the current value
    m_theme_button = new wxButton(this, wxID_ANY);
    if (m_darkmode) {
        m_theme_button->SetBitmap(LoadEmbeddedSVG("Assets/dark.svg", wxSize(48,48)));
    }
    else {
        m_theme_button->SetBitmap(LoadEmbeddedSVG("Assets/light.svg", wxSize(48,48)));
    }
    mainSizer->Add(m_theme_button, 0, wxALL, 10);
    //add a binding to handle the button being pressed
    m_theme_button->Bind(wxEVT_BUTTON, &ProjectConfigDialog::onToggleTheme, this);

    //define the editable field for entering the sample rate
    sampleSizer->Add(new wxStaticText(this, wxID_ANY, "Sample Rate:"), 1, wxALL, 10);
    sample_rate_field = new wxTextCtrl(this, wxID_ANY);
    sampleSizer->Add(sample_rate_field, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    //create the button for confirming changes to the sample rate
    sample_rate_confirm = new wxButton(this, wxID_ANY, "Confirm");
    //add the button to the sampleSizer
    sampleSizer->Add(sample_rate_confirm, 1, wxALL, 10);
    //bind the event handler to the confirm button
    sample_rate_confirm->Bind(wxEVT_BUTTON, &ProjectConfigDialog::onNewSampleRate, this);

    //finally, add the sensor sample rate config menu
    mainSizer->Add(sampleSizer, 0, wxEXPAND | wxALL, 10);

    //create a sizer for the selection and confirmation of which sensor reading to use
    wxBoxSizer* getterSizer = new wxBoxSizer(wxHORIZONTAL);

    //create and add the text control for the getter selection
    getterSizer->Add(new wxStaticText(this, wxID_ANY, "Select Reading:"), 1, wxALL, 10);

    //create and add the selection box:
    getterSelection = new wxComboBox(this, wxID_ANY);
    getterSelection->Append("Raw");
    getterSelection->Append("Voltage");
    getterSelection->Append("Mapped");
    getterSizer->Add(getterSelection, 1, wxALL, 10);

    //create and add the button to confirm the getter method selection
    confirm_getter = new wxButton(this, wxID_ANY, "Confirm");
    getterSizer->Add(confirm_getter, 1, wxALL, 10);
    //bind the button to the event handler
    confirm_getter->Bind(wxEVT_BUTTON, &ProjectConfigDialog::onChangeReadStrategy, this);

    mainSizer->Add(getterSizer, 0, wxALL, 10);

    // OK / Cancel buttons
    wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    btnSizer->AddButton(new wxButton(this, wxID_OK));
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
    btnSizer->Realize();

    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
}

void ProjectConfigDialog::onChangeReadStrategy(wxCommandEvent& event) {

    //first, make sure that the user actually selected something:
    int selection = getterSelection->GetSelection();
    if (selection == wxNOT_FOUND){
        wxLogMessage("No option selected!\nPlease select a port from the drop-down and try again.");
        return;
    }

    //get the pointer to the sensor manager for the current project
    m_sensorManager = m_project->getSensorManager();

    const std::string string_selection{getterSelection->GetStringSelection()};
    m_sensorManager->setAllReadingStrategy(string_selection);
}

void ProjectConfigDialog::onNewSampleRate(wxCommandEvent& evt) {

    //check to make sure we've actually connected with a microcontroller
    if (!m_project->handshakeComplete) {
        wxMessageBox("Please Connect to a Microcontroller before adjusting sample rate",
        "Error",
        wxOK | wxICON_ERROR,
        this);
        sample_rate_field->SetFocus();
        return;
    }

    wxString sample_rate = sample_rate_field->GetValue();

    // Trim whitespace from both ends
    sample_rate.Trim(true).Trim(false);

    // Check if empty
    if (sample_rate.IsEmpty())
    {
        wxMessageBox("Please enter a value.",
                     "Input Error",
                     wxOK | wxICON_ERROR,
                     this);
        sample_rate_field->SetFocus();
        return;
    }

    int value;
    if (!sample_rate.ToInt(&value))
    {
        wxMessageBox("Please enter a valid integer.",
                     "Input Error",
                     wxOK | wxICON_ERROR,
                     this);
        sample_rate_field->SetFocus();
        return;
    }

    //now that we've validated the input, we can apply the changes to the project
    m_project->adjustSampleRate(value);
}

void ProjectConfigDialog::onToggleTheme(wxCommandEvent& evt) {
    wxCommandEvent event(wxEVT_THEME_TOGGLE);          // custom event
    wxPostEvent(this, event);    // send to MainFrame

    // Update button icon locally for feedback
    if (m_darkmode)
        m_theme_button->SetBitmap(LoadEmbeddedSVG("Assets/light.svg", wxSize(48,48)));
    else
        m_theme_button->SetBitmap(LoadEmbeddedSVG("Assets/dark.svg", wxSize(48,48)));

    m_darkmode = !m_darkmode;
}