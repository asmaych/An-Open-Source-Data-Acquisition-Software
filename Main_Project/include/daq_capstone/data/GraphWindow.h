#pragma once
#include <wx/wx.h>
#include "DataSession.h"
#include <wx/graphics.h>
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
		GraphWindow(wxWindow* parent, const std::vector<double>& timestamps, 
				const std::vector<double>& values, const std::string& sensorName);

		
	private:
		void OnPaint(wxPaintEvent& evt);
		void drawGraph(wxDC& dc);

		std::vector<double> m_timestamps;
		std::vector<double> m_values;
		std::string m_sensorName; //Collected data to be graphed
		wxPanel* m_panel;

		wxDECLARE_EVENT_TABLE();
};
