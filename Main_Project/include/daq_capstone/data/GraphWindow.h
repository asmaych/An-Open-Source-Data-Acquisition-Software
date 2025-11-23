#pragma once
#include <wx/wx.h>
#include "DataSession.h"

/* Graphwindow is a class taht displays a graph of the collected data.
   each GraphWIndow is tied to a DataSession and can display/visualize its values.
   Currently just a placeholder till i make sure everything works perfectly i will implement it
*/

class GraphWindow : public wxFrame
{
	public:
		//constructor with a parent with is the mainFrame and a pointer to DataSession
		GraphWindow(wxWindow* parent, DataSession* session);
	private:
		DataSession* m_session; //Collected data to be graphed
};
