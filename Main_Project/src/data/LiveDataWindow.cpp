#include "data/LiveDataWindow.h"

//Constructor: sets up the GUI window
LiveDataWindow::LiveDataWindow(wxWindow* parent, const std::vector<Sensor*>& activeSensors)
	: wxFrame(parent, wxID_ANY, "Live Data", wxDefaultPosition, wxSize(400, 300)), 
	  m_timer (this),
	  m_sensors(activeSensors)
{
	/* Create a multi-line read-only text box
	   wxTE_MULTILINE: allows multiple lines
	   wxTE_READONLY: prevents user from editing the display (since we are allowing read-only)
	*/
	m_display = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

	Bind(wxEVT_TIMER, &LiveDataWindow::onFlush, this);

	//std::cout << "Sensors: " << m_sensors.size() << std::endl;

	m_timer.Start(100);

}

//Add a value to display and save it in session for later collecting/graphing
void LiveDataWindow::addValue(double value)
{
	//buffer values instead of updating GUI immediately
	m_buffer.push_back(value);
}

void LiveDataWindow::onFlush(wxTimerEvent&)
{
	if(m_sensors.empty())
		return;

	//temporarily freeze UI updates
	m_display -> Freeze();

	//combine all buffered values into one string
	wxString row;

	for (size_t i = 0; i < m_sensors.size(); ++i)
	{
		//append current sensor value
		row += wxString::Format("%.2f", static_cast<double> (m_sensors[i] -> getReading()));

		//std::cout << i << ": " << m_sensors[i] -> getReading() << std::endl;

		//if there still values to display, seperate them by a comma
		if((i+1) < m_sensors.size()){
			row += ", ";
		}
	}
	//otherwise if its the value of the last sensor, then new line
        row += "\n";
	//std::cout << "Row: " << row.ToStdString() << std::endl;


	//append all formatted frames to the text control at once
	m_display -> AppendText(row);

	//limit total number of lines to avoid performance degradation
	const long maxLines = 2000;
	long lines = m_display -> GetNumberOfLines();

	if(lines > maxLines){
		//remove oldest lines from the beginning
		long pos = m_display -> XYToPosition(0, lines - maxLines);
		m_display -> Remove(0, pos);
	}

	//re-enable UI updates
	m_display -> Thaw();
}

//clear
void LiveDataWindow::clearDisplay()
{
	if(m_display){
		m_display -> Clear();
	}
}
