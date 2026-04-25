#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);
bool App::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler());

        MainFrame* mainFrame = new MainFrame("OSdaq");

        //Set the mainFrame visibility to true (false by default)
        mainFrame->Show();

	//set the main GUI to a default size
	mainFrame->SetClientSize(800, 600);

	mainFrame->Center();

        return true;
}

