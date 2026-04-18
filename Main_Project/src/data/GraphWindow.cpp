#include "data/GraphWindow.h"
#include <algorithm>
#include <map>
#include <wx/checklst.h>
#include <wx/sizer.h>

/* This constructor creates a window that will diplay a graph :
	parent: the window that owns this graph (projectX)
*/

static wxColour COLORS[] = { *wxRED, *wxBLUE, *wxGREEN, *wxCYAN, *wxYELLOW, *wxLIGHT_GREY,  *wxWHITE};

GraphWindow::GraphWindow(wxWindow* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	// ========== Graph panel ==========
	m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	m_panel -> SetMinSize(wxSize(200,150));
	m_panel -> Bind(wxEVT_PAINT, &GraphWindow::OnPaint, this);

	sizer -> Add(m_panel, 1, wxEXPAND | wxALL, 5);

	// ========== SELECTOR BUTTON =======
	m_selectedButton = new wxButton(this, wxID_ANY, "Runs");
	sizer -> Add(m_selectedButton, 0, wxALIGN_RIGHT | wxALL, 5);

	m_selectedButton -> Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
		openCurveSelector();
	});

	SetSizer(sizer);
}


// ===================== ADD CURVE ======================
//adds a new curve to the graph
void GraphWindow::addCurve(const std::vector<double>& x, const std::vector<double>& y, const std::string& label,
				size_t runNumber, const std::string& id)
{
	std::lock_guard<std::mutex> lock(m_graphMutex);

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
	c.visible = true; //should the curve get displayed or not
	c.runNumber = runNumber; //to which run does it belong

	m_curves.push_back(c);  //store the curve
	m_panel -> Refresh();  //trigger repaint of panel
}


// ===================== CLEAR CURVES ====================
//clears all curves from the graph
void GraphWindow::clear()
{
	std::lock_guard<std::mutex> lock(m_graphMutex);
	m_curves.clear();  //remove all curves
	m_panel -> Refresh();  //refresh panel to erase drawing
}


// ==================== PAINT HANDLER ===================
//called whenever the panel needs to be repainted
void GraphWindow::OnPaint(wxPaintEvent& evt)
{
	//for drawing
	wxPaintDC dc(m_panel);

	int w, h;
    	m_panel -> GetClientSize(&w, &h);
    	if(w <= 0 || h <= 0)
		return;

    	//draw into an off-screen bitmap first, no flicker because the panel is never shown blank between erase and redraw(fingers crossed)
    	wxBitmap buffer(w, h);
    	wxMemoryDC memDC(buffer);

    	//fill background
    	//wxColour bg = (m_currentTheme == Theme::Dark) ? wxColour(30, 30, 30) : *wxWHITE;
    	memDC.SetBackground(wxBrush(m_panel -> GetBackgroundColour()));
    	memDC.Clear();

    	//draw everything into the off-screen buffer
    	draw(memDC);

    	//blit the completed frame to screen in one atomic operation
    	dc.Blit(0, 0, w, h, &memDC, 0, 0);
}


// ==================== DRAW FUNCTION ===================
//draws all curves
void GraphWindow::draw(wxDC& dc)
{
	std::lock_guard<std::mutex> lock(m_graphMutex);
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
		//if nothing is selected or no data is collected skip
        	if(!c.visible || c.x.empty())
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

	//if flat line, do not skip it and artificially expand range
	if (minY == maxY){
		minY -= 1;
		maxY += 1;
        }
	if(minX == maxX){
		minX -= 1;
		maxX += 1;
	}

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
		if(!c.visible)
			continue;

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

		//draw collect now points
		for(auto& point : c.demandPoints)
		{
			int px = left + (point.first - minX) / (maxX - minX) * (right - left);
			int py = bottom - (point.second - minY) / (maxY - minY) * (bottom - top);

			//using a strong visible color
			wxColour highlight(255, 80, 80);

			dc.SetPen(wxPen(highlight, 2));
			dc.SetBrush(wxBrush(highlight));

			//draw the highlighted point
			dc.DrawCircle(px, py, 4);
		}
   	}

    	// ======== Draw legend ========
    	int ly = top;
    	for (auto& c : m_curves)
    	{
		if(!c.visible)
			continue;
        	dc.SetTextForeground(c.color);
        	dc.DrawText(c.label, right - 120, ly);
        	ly += 15;
    	}
}

//draw & add the on demand pointss to the curve
void GraphWindow::addDemandPoint(const std::string& curveId, double x, double y)
{
	std::lock_guard<std::mutex> lock(m_graphMutex);

	bool found = false;
	for(auto& curve : m_curves){
		if(curve.id == curveId){
			curve.demandPoints.emplace_back(x,y);
			found = true;
			std::cout << "added demand point to curve " << curveId << "at(" << x <<"," << y <<")\n";
			break;
		}
	}
	if(!found)
		std::cout << "Curve ID not found: " << curveId << "\n";

	m_panel -> Refresh();
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

	//create an off screen bitmap with the size of the panel
	wxBitmap bitmap(w, h);

	//create a memory device context to draw into the bitmap
	wxMemoryDC memDC(bitmap);

	//set background color based on the current theme
	memDC.SetBackground(wxBrush(m_panel -> GetBackgroundColour()));

	//clear the bitmap using the backgrounf brush
	memDC.Clear();

	//draw the graph exactly like on screen
	draw(memDC);

	//detach the bitmap from the memory dc which is required before saving the bitmap as a file
	memDC.SelectObject(wxNullBitmap);

	//save the bitmap as a png image
	bitmap.SaveFile(path, wxBITMAP_TYPE_PNG);
}


// =============== CURVE SELECTOR DIALOG ===============
//on click, shows a checklist of all the runs & their sensors that exist with a select/deselect all buttons
void GraphWindow::openCurveSelector()
{
	// no curve? nothing to select
    	if (m_curves.empty())
        	return;

    	//modal dialog (blocks entire app until the user closes it)
    	wxDialog dialog(this, wxID_ANY, "Select Runs / Sensors", wxDefaultPosition, wxSize(350, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	//main vertical sizer for the dialog layout
    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//temporary map to group curves by run number
	std::map<size_t, std::vector<Curve*>> runByNumber;
	for(auto& curve : m_curves)
		runByNumber [curve.runNumber].push_back(&curve);

	//keep track of all checkboxes and corresponding curves for select/Deselect all buttons
	//storing every checkbox and its corresponding curve makes it easy for select/deselect all to toggle both the checkbox & visibility
	std::vector<std::pair<wxCheckBox*, Curve*>> bindings;

	//build the UI: a label for each run and checkboxes for its sensors
    	for (auto& [runNumber, curves] : runByNumber)
	{
		//bold label showing the run number to improve readability, especially with multiple runs
    		wxStaticText* runLabel = new wxStaticText(&dialog, wxID_ANY, wxString::Format("Run %zu", runNumber));
		wxFont font = runLabel->GetFont();
    		font.SetWeight(wxFONTWEIGHT_BOLD);
    		runLabel->SetFont(font);

    		mainSizer->Add(runLabel, 0, wxTOP | wxLEFT, 10);

        	//add checkboxes for each sensor(curve) under the run
        	for (auto* curve : curves)
        	{
            		wxCheckBox* cb = new wxCheckBox(&dialog, wxID_ANY, curve->label);

			//initialize checkbox to the current visibility state of the curve
            		cb->SetValue(curve->visible);

			//bind checkbox event: when checked/unchecked, update curve visibility and refresh the panel
            		cb->Bind(wxEVT_CHECKBOX, [this, curve](wxCommandEvent& evt){
				curve->visible = evt.IsChecked();
                    		m_panel -> Refresh();
                	});

			//store checkbox and curve pointer for later use (select/deselect all)
    	        	bindings.emplace_back(cb, curve);

			//indent sensors under the run label
        	    	mainSizer->Add(cb, 0, wxLEFT, 30);
        	}
    	}

    	//horizontal sizer for the buttons
    	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

	//buttons for select/deselect all & OK
    	wxButton* selectAll = new wxButton(&dialog, wxID_ANY, "Select All");
    	wxButton* deselectAll = new wxButton(&dialog, wxID_ANY, "Deselect All");
    	wxButton* ok = new wxButton(&dialog, wxID_OK, "OK");

	//bind select all button: set all checkboxes and curves to visible
    	selectAll->Bind(wxEVT_BUTTON,[this, &bindings](wxCommandEvent&){
            	for (auto& [cb, curve] : bindings)
            	{
                	cb->SetValue(true);
                	curve->visible = true;
            	}
            	Refresh();
        	});

	//do the same for deselect all but vice versa
    	deselectAll->Bind(wxEVT_BUTTON,[this, &bindings](wxCommandEvent&){
            	for (auto& [cb, curve] : bindings)
            	{
                	cb->SetValue(false);
                	curve->visible = false;
            	}
            	Refresh();
        	});

	//add buttons to the horizontal sizer with spacing
    	btnSizer->Add(selectAll, 0, wxRIGHT, 5);
    	btnSizer->Add(deselectAll, 0, wxRIGHT, 5);
    	btnSizer->AddStretchSpacer();
    	btnSizer->Add(ok, 0);

	//add spacing and the button row to the main sizer
    	mainSizer->AddSpacer(10);
    	mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 10);

	//apply the sizer to the dialog and fit it to content
    	dialog.SetSizerAndFit(mainSizer);
    	dialog.Centre();

	//show the dialog modally, the user must close it before returning to main window
    	dialog.ShowModal();
}
