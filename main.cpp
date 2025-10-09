// main.cpp
#include <wx/wx.h>
#include <vector>
#include <string>
#include "SerialComm.h"


class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 200))
    {
        wxPanel* panel = new wxPanel(this, wxID_ANY);

        wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

        // Label
        wxStaticText* label = new wxStaticText(panel, wxID_ANY, "Select a Serial Port:");
        vbox->Add(label, 0, wxALL | wxCENTER, 10);

        // Combo box
        ports = comm.getPorts();
        if (ports.empty()) {
            wxMessageBox("No serial ports found.", "Error", wxOK | wxICON_ERROR);
            Close();
            return;
        }

        wxArrayString choices;
        for (const auto& port : ports) {
            choices.Add(port);
        }

        combo = new wxComboBox(panel, wxID_ANY, choices[0], wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
        vbox->Add(combo, 0, wxALL | wxEXPAND, 10);

        // Button
        wxButton* btn = new wxButton(panel, wxID_ANY, "Connect");
        vbox->Add(btn, 0, wxALL | wxCENTER, 10);

        panel->SetSizer(vbox);

        // Event binding
        btn->Bind(wxEVT_BUTTON, &MyFrame::OnConnect, this);
    }

private:
    SerialComm comm;
    std::vector<std::string> ports;
    wxComboBox* combo;

    void OnConnect(wxCommandEvent& event)
    {
        wxString selection = combo->GetValue();
        std::string port = selection.ToStdString();

        if (!comm.handshake(port)) {
            wxMessageBox("Handshake failed.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        if (!comm.cleanPort()) {
            wxMessageBox("Failed to clean up port.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        wxMessageBox("Handshake successful!", "Success", wxOK | wxICON_INFORMATION);
    }
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        MyFrame* frame = new MyFrame("Serial Port Selector");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);








// // main.cpp
// #include <iostream>
// #include <vector>
// #include <string>
// #include "SerialComm.h"
//
// int main()
// {
//     SerialComm comm;
//
//     const std::vector<std::string> &ports = comm.getPorts();
//     if (ports.empty()) {
//         std::cerr << "No serial ports found." << std::endl;
//         return 1;
//     }
//
//     // Allow user to select port
//     std::cout << "Select a port: \n ";
//
//     for (std::string choice : ports)
//     {
//     	std::cout << choice << "\n";
//     }
//
//     //now allow the user to enter a port:
//     std::string portchoice;
//     std::cin >> portchoice;
//
//     // Call the provided handshake routine which performs the send/receive.
//     comm.handshake(portchoice);
//
//     // Clean up / close port using the provided function.
//     if (!comm.cleanPort()) {
//         std::cerr << "cleanPort() reported failure." << std::endl;
//         return 2;
//     }
//
//     return 0;
// }
//
