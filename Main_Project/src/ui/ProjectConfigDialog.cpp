#include "ProjectConfigDialog.h"
#include "ProjectPanel.h"

ProjectConfigDialog::ProjectConfigDialog(wxWindow* parent,
                                         ProjectPanel* project)
    : wxDialog(parent,
               wxID_ANY,
               "Project Configuration",
               wxDefaultPosition,
               wxSize(400, 300),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_project(project)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    //sizer for the sensor sample rate configuration
    wxBoxSizer* sampleSizer = new wxBoxSizer(wxHORIZONTAL);

    // Example setting: Theme toggle
    m_theme_toggle = new wxCheckBox(this, wxID_ANY, "Enable Dark Mode");
    mainSizer->Add(m_theme_toggle, 0, wxALL, 10);

    //define the editable field for entering the sample rater
    sampleSizer->Add(new wxStaticText(this, wxID_ANY, "Sample Rate:"), 0, wxALL, 5);
    sample_rate_field = new wxTextCtrl(this, wxID_ANY);
    sampleSizer->Add(sample_rate_field, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    //create the button for confirming changes to the sample rate
    sample_rate_confirm = new wxButton(this, wxID_ANY, "Confirm");
    //add the button to the sampleSizer
    sampleSizer->Add(sample_rate_confirm, 0, wxALL, 10);

    //finally, add the sensor sample rate config menu
    mainSizer->Add(sampleSizer, 0, wxEXPAND | wxALL, 10);

    // OK / Cancel buttons
    wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    btnSizer->AddButton(new wxButton(this, wxID_OK));
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
    btnSizer->Realize();

    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);

    //------------------------------------------------------------------------------------------------------------------
    //BIND EVENTS
    //------------------------------------------------------------------------------------------------------------------
    sample_rate_confirm->Bind(wxEVT_BUTTON, &ProjectConfigDialog::onNewSampleRate, this);

    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt)
{
    EndModal(wxID_CANCEL);
});

    // Handle OK click
    Bind(wxEVT_BUTTON, [this](wxCommandEvent& evt)
    {
        if(evt.GetId() == wxID_OK)
        {
            dark = m_theme_toggle->GetValue();
            EndModal(wxID_OK);
        }
    });
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