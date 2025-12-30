#include "data/GraphWindow.h"
#include <algorithm>

// These marcos declare that this class has an event table (a wxwidget system that maps events like paint to functionsà

wxBEGIN_EVENT_TABLE(GraphWindow, wxFrame)

wxEND_EVENT_TABLE()

/* This constructor creates a window that will diplay a graph :
	parent: the window that owns this graph (projectX)
	timestamps: X-axis values (time)
	values: Y-axis values (sensor data)
	sensorName: used as the window title
*/

GraphWindow::GraphWindow(wxWindow* parent, const std::vector<double>& timestamps, const std::vector<double>& values,
			const std::string& sensorName)
	: wxFrame(parent, wxID_ANY, wxString(sensorName), wxDefaultPosition, wxSize(600, 400)), 
          m_timestamps(timestamps), m_values (values), m_sensorName(sensorName)
{
	m_panel = new wxPanel(this, wxID_ANY);
	m_panel -> Bind(wxEVT_PAINT, &GraphWindow::OnPaint, this);
}

void GraphWindow::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(m_panel);
	dc.Clear();
	drawGraph(dc);
}

void GraphWindow::drawGraph(wxDC&dc)
{
	if(m_timestamps.empty() || m_values.empty())
		return;

	int w,h;
	m_panel -> GetSize(&w, &h);

	//we need min and max to normalize
	double minX = m_timestamps.front(); 
        double maxX = m_timestamps.back();

	if(maxX == minX)
		return;

	double minY = *std::min_element(m_values.begin(), m_values.end());
	double maxY = *std::max_element(m_values.begin(), m_values.end());

	if(maxY == minY)
		maxY += 1;

	dc.SetPen(*wxBLUE_PEN);

	//we draw the lines
	for(size_t i = 1; i < m_values.size(); ++i){
		int x1 = (int) ((m_timestamps[i - 1] - minX) / (maxX - minX) * w);
		int y1 = h - (int) ((m_values[i - 1] - minY) / (maxY - minY) * h);
		int x2 = (int)((m_timestamps[i] - minX) / (maxX - minX) * w);
		int y2 = h - (int)((m_values[i] - minY) / (maxY - minY) * h);

		dc.DrawLine(x1,y1,x2,y2);
	}
}
