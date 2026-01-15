#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>

/* Graphwindow is a class taht displays a graph of the collected data.
   each GraphWIndow is tied to a DataSession and can display/visualize its values.
   Currently just a placeholder till i make sure everything works perfectly i will implement it
*/

class GraphWindow : public wxFrame
{
	public:
		//constructor with a parent with is the mainFrame and a pointer to DataSession
		GraphWindow(wxWindow* parent);

		//Add one curve (one sensor from one run)
		void addCurve(const std::vector<double>& x, const std::vector<double>& y,
			      const std::string& label, const std::string& id);

		//remove all curves
		void clear();

	private:
		struct Curve{ std::vector<double> x; std::vector<double> y; std::string label;
				wxColour color; std::string id; }; //id is a unique key: runID

		wxPanel* m_panel;
		std::vector<Curve> m_curves;

		void OnPaint(wxPaintEvent& evt);
		void draw(wxDC& dc);

		wxDECLARE_EVENT_TABLE();
};
