#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <map>
#include "ui/Theme.h"
#include "ui/ProjectPanel.h"

enum class Theme;

/**
 * @brief GUI object that displays a graph based on vectorized datapoints
 *
 * Graphwindow is a class that displays a graph of the collected sensor data.
 * each GraphWindow is tied to a DataSession and can display/visualize its values.
*/
class GraphWindow : public wxPanel
{
	public:
		/**
		 * @brief Collection of data elements used to generate a plot of readings on a GraphWindow
		 */
		struct Curve{	std::vector<double> x;
						std::vector<double> y;
						std::string label;
						wxColour color;
						std::string id;
						bool visible = true;
						size_t runNumber;
						std::vector<std::pair<double,double>> demandPoints; }; //id is a unique key: runID

		//constructor with a parent with is the mainFrame and a pointer to DataSession
		GraphWindow(wxWindow* parent);

		//Add one curve (one sensor from one run)
		void addCurve(const std::vector<double>& x, const std::vector<double>& y, const std::string& label,
				size_t runNumber, const std::string& id);

		//draw the collect on demand points
		void addDemandPoint(const std::string& curveId, double x, double y);

		//remove all curves
		void clear();

		//change theme for the graph
		void setTheme(Theme theme);

		//export the graph as png
		void exportImage(const wxString& path);

		//getter
		const std::vector<Curve>& getCurves() const { return m_curves; }

	private:
		wxPanel* m_panel;
		std::vector<Curve> m_curves;
		Theme m_currentTheme = Theme::Light;
		wxButton* m_selectedButton; //Selector button for showing/hiding curves
		std::mutex m_graphMutex;

		void OnPaint(wxPaintEvent& evt);
		void draw(wxDC& dc);

		void openCurveSelector(); //open dialog for curve visibility

};
