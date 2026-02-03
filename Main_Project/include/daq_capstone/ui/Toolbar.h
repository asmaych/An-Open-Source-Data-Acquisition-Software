#pragma once
#include <wx/wx.h>
#include <functional>
#include "Theme.h"

/*
	Toolbar is a UI component that owns buttons and visual state and that calls back to Mainframe
*/

class Toolbar
{
	public:

		enum ToolID {
				ID_StartStop = wxID_HIGHEST + 100,
				ID_Reset,
				ID_CollectNow,
				ID_Graph,
				ID_Export,
				ID_Theme
		};

		Toolbar(wxFrame* parent);

		//callbacks
		std::function<void()> onStartStop;
		std::function<void()> onReset;
		std::function<void()> onCollectNow;
		std::function<void()> onGraph;
		std::function<void()> onExport;
		std::function<void()> onToggleTheme;

		//visual state
		void bindEvents(wxFrame* parent);
		void setRunning(bool running);
		void applyTheme(Theme theme);

		//getters
		wxToolBar* get();
		wxStaticText* getTimerDisplay() { return m_timerDisplay; } //getter for timer

	private:

		wxToolBar* m_toolbar = nullptr;
		wxStaticText* m_timerDisplay = nullptr;
		bool m_isRunning = false;

};
