#include "data/GraphWindow.h"

GraphWindow::GraphWindow(wxWindow* parent, DataSession* session)
	: wxFrame(parent, wxID_ANY, "Graph", wxDefaultPosition, wxSize(600, 400)), m_session(session)
{
	//TODO: implement the actual data plotting using wxCharts
}
