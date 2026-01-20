#include "data/LiveDataWindow.h"

//Constructor: sets up the GUI window
LiveDataWindow::LiveDataWindow(wxWindow* parent)
	: wxFrame(parent, wxID_ANY, "Live Data", wxDefaultPosition, wxSize(400, 300))
{
	// Notebook allows multiple runs as tabs
	m_notebook = new wxNotebook(this, wxID_ANY);

	//catch the close event
	Bind(wxEVT_CLOSE_WINDOW, &LiveDataWindow::OnClose, this);
}

//creates a new tab and marks it as active
void LiveDataWindow::startNewRun(std::shared_ptr<Run> run)
{
	m_activeRun = run;
	wxString title = wxString::Format("Run %zu", run -> getRunNumber());

	m_activeTextCtrl = new wxTextCtrl(m_notebook, wxID_ANY, "",  wxDefaultPosition, wxDefaultSize, 
					  wxTE_MULTILINE | wxTE_READONLY);

	m_activeTextCtrl -> AppendText("Time : Sensor Values\n");

	m_notebook -> AddPage(m_activeTextCtrl, title, true);
}

//prevents further writing to the run
void LiveDataWindow::stopRun()
{
	m_activeRun.reset();
	m_activeTextCtrl = nullptr;
}

//append one frame to the currently active run
void LiveDataWindow::addFrame(double time, const std::vector<double>& values)
{
	if(!m_activeTextCtrl)
		return;

	wxString line;
	line << wxString::Format("%.3f : ", time);

	for(size_t i = 0; i < values.size(); ++i){
		line << wxString::Format("%.2f", values[i]);
		if (i+1 < values.size())
			line << ", ";
	}

	line << "\n";
	m_activeTextCtrl -> AppendText(line);
}

void LiveDataWindow::clearAll()
{
	m_notebook -> DeleteAllPages();
	m_activeRun.reset();
	m_activeTextCtrl = nullptr;
}

void LiveDataWindow::OnClose(wxCloseEvent& evt)
{
	//stop live updating
	m_activeRun.reset();
	m_activeTextCtrl = nullptr;

	//Im using hide instead of destroy to not stop live updating of graph (TBD)
	Hide();
}
