#pragma once
#include <wx/wx.h>

/**
 * @brief This is the launch point for the entire application. It provides no logical operations.
 */
class App : public wxApp
{
	public:
		///
		///	@brief This is the launching point of the application GUI
		///
		///	This method is used to initialize the wxWidgets app. After executing, the main GUI panel MainFrame.cpp will be
		///	launched.
		///
		/// @return bool
		bool OnInit();
};
