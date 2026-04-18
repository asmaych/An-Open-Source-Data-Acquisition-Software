#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);
bool App::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler());

    	setenv("GTK_THEME", "Adwaita:dark", 1);

        MainFrame* mainFrame = new MainFrame("Testing main gui");

        //Set the mainFrame visibility to true (false by default)
        mainFrame->Show();

	//set the main GUI to a default size
	mainFrame->SetClientSize(800, 600);

	mainFrame->Center();

        return true;
}

