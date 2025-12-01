#include "data/LiveDataWindow.h"

//Constructor: sets up the GUI window
LiveDataWindow::LiveDataWindow(wxWindow* parent, DataSession* session)
	: wxFrame(parent, wxID_ANY, "Live Data", wxDefaultPosition, wxSize(400, 300)), m_session(session)
{
	/* Create a multi-line read-only text box
	   wxTE_MULTILINE: allows multiple lines
	   wxTE_READONLY: prevents user from editing the display (since we are allowing read-only)
	*/
	m_display = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
}

//display a raw string (comma-seperated sensor readings)
void LiveDataWindow::appendBuffer(const std::string& buffer)
{
	if(m_display){
		m_display -> AppendText(buffer + "\n");
	}
}

//Add a value to display and save it in session for later collecting/graphing
void LiveDataWindow::addValue(double value)
{
	if(m_session){
		m_session -> addValue(value);
	}
	
	//display value in GUI
	if (m_display) {
		m_display -> AppendText(wxString::Format("%.2f\n", value));
	}
}

void LiveDataWindow::clearDisplay()
{
	if(m_display){
		m_display -> Clear();
	}
}
