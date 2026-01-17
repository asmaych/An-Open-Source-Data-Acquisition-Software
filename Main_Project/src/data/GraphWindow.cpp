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

static wxColour COLORS[] = { *wxRED, *wxBLUE, *wxGREEN, *wxCYAN, *wxYELLOW, *wxLIGHT_GREY,  *wxWHITE};

GraphWindow::GraphWindow(wxWindow* parent)
	: wxFrame(parent, wxID_ANY, "Graph", wxDefaultPosition, wxSize(700, 500))
{
	m_panel = new wxPanel(this);
	m_panel -> Bind(wxEVT_PAINT, &GraphWindow::OnPaint, this);
}


// ===================== ADD CURVE ======================
//adds a new curve to the graph
void GraphWindow::addCurve(const std::vector<double>& x, const std::vector<double>& y,
			   const std::string& label, const std::string& id)
{
	//check if the curve already exists
	for(auto& c : m_curves){
		if(c.id == id){
			//updates values only, keep color and label the same
			c.x = x;
			c.y = y;
			m_panel -> Refresh();
			return;
		}
	}

	//otherwise create a new curve
	Curve c;
	c.id = id;
	c.x = x;  //x values of the curve
	c.y = y;  //y values of the curve
	c.label = label;  //label for legend
	c.color = COLORS[m_curves.size() % (sizeof(COLORS)/sizeof(COLORS[0]))];  //pick color cyclically from palette

	m_curves.push_back(c);  //store the curve
	m_panel -> Refresh();  //trigger repaint of panel
}


// ===================== CLEAR CURVES ====================
//clears all curves from the graph
void GraphWindow::clear()
{
	m_curves.clear();  //remove all curves
	m_panel -> Refresh();  //refresh panel to erase drawing
}


// ==================== PAINT HANDLER ===================
//called whenever the panel needs to be repainted
void GraphWindow::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(m_panel); //for drawing
	dc.Clear(); //clear panel background
	draw(dc);
}


// ==================== DRAW FUNCTION ===================
//draws all curves
void GraphWindow::draw(wxDC& dc)
{
	//if there is no data
	if (m_curves.empty())
        	return;

	//get drawable area size
    	int w, h;
    	m_panel->GetSize(&w, &h);

    	// ======== Layout margins ========
    	//leave space for axis labels and legend
	int left   = 60; //space for y_axis labels
    	int right  = w - 20; //right boundary of graph
    	int top    = 20; //top padding
    	int bottom = h - 50; //space for X axis labels

    	// ======== Find global min/max for scaling ========
	//we scan all curves to find the full data range
    	double minX = 1e9, maxX = -1e9;
    	double minY = 1e9, maxY = -1e9;

    	for (auto& c : m_curves)
    	{
        	if (c.x.empty()) 
			continue;

		//x range uses time values
        	minX = std::min(minX, c.x.front());
        	maxX = std::max(maxX, c.x.back());

		//y range scans all sensor values
        	for (double v : c.y)
        	{
            	minY = std::min(minY, v);
            	maxY = std::max(maxY, v);
        	}
    	}

	//if there is no variation, we cannot scale or draw
    	if (minX == maxX || minY == maxY)
        	return;

	// ====== Theme colors ======
	wxColour axisColor, labelColor;
	if(m_currentTheme == Theme::Dark){
		axisColor = *wxWHITE;
		labelColor = *wxWHITE;
	} else {
		axisColor = *wxBLACK;
		labelColor = *wxBLACK;
	}

    	// ======== Draw axes ========
    	dc.SetPen(wxPen(axisColor, 2)); //(color, width)

    	// Y axis
    	dc.DrawLine(left, top, left, bottom);

    	// X axis
    	dc.DrawLine(left, bottom, right, bottom);

    	// ======== Draw ticks & labels ========
    	int ticks = 5; //number of divisions on each axis

    	for (int i = 0; i <= ticks; i++)
    	{
		//t goes from 0 to 1
        	double t = (double)i / ticks;

		//convert normalized position to screen coordinates
        	int y = bottom - t * (bottom - top);
        	int x = left + t * (right - left);

		//convert normalized position to real data values
        	double yVal = minY + t * (maxY - minY);
        	double xVal = minX + t * (maxX - minX);

        	// Y ticks
        	dc.DrawLine(left - 5, y, left, y);
		dc.SetTextForeground(labelColor);
        	dc.DrawText(wxString::Format("%.2f", yVal), 5, y - 7);

        	// X ticks
        	dc.DrawLine(x, bottom, x, bottom + 5);
        	dc.DrawText(wxString::Format("%.2f", xVal), x - 20, bottom + 10);
    	}

    	// ======== Draw curves ========
    	for (auto& c : m_curves)
    	{
		//each curve gets its own color
        	dc.SetPen(wxPen(c.color, 2));

		//draw line segmnents between consecutive points
        	for (size_t i = 1; i < c.x.size(); ++i)
        	{
			//we convert data coordinates to screen coordinates
            		int x1 = left + (c.x[i - 1] - minX) / (maxX - minX) * (right - left);
            		int y1 = bottom - (c.y[i - 1] - minY) / (maxY - minY) * (bottom - top);

            		int x2 = left + (c.x[i] - minX) / (maxX - minX) * (right - left);
            		int y2 = bottom - (c.y[i] - minY) / (maxY - minY) * (bottom - top);

			//draw the line segment
            		dc.DrawLine(x1, y1, x2, y2);
        	}
    	}

    	// ======== Draw legend ========
    	int ly = top;
    	for (auto& c : m_curves)
    	{
        	dc.SetTextForeground(c.color);
        	dc.DrawText(c.label, right - 120, ly);
        	ly += 15;
    	}
}

//store the current theme (all the updates will be made in draw)
void GraphWindow::setTheme(Theme theme)
{
	m_currentTheme = theme;
	m_panel -> SetBackgroundColour(theme == Theme :: Dark ? wxColour(30,30,30) : *wxWHITE);
	m_panel -> Refresh(); //redraw graph with new theme
}

//export the graph as png
void GraphWindow::exportImage(const wxString& path)
{
	if(m_curves.empty())
		return;

	//get the current size of graph panel to ensure that the exported image matches on screen size
	int w,h;
	m_panel -> GetSize(&w, &h);

	//create an off screen bitmap  with the same size as the panel
	wxBitmap bitmap(w, h);

	//create a memory device context to draw into the bitmap
	wxMemoryDC memDC(bitmap);

	//set background color based on the current theme
	memDC.SetBackground(m_currentTheme == Theme::Dark ? wxBrush(wxColour(30,30,30)) : wxBrush(*wxWHITE));

	//clear the bitmap using the backgrounf brush
	memDC.Clear();

	//draw the graph exactly like on screen
	draw(memDC);

	//detach the bitmap from the memory dc which is required before saving the bitmap as a file
	memDC.SelectObject(wxNullBitmap);

	//save the bitmap as a png image
	bitmap.SaveFile(path, wxBITMAP_TYPE_PNG);
}
