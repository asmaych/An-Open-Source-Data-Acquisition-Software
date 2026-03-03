#pragma once
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <functional>
#include <wx/timer.h>
#include "Theme.h"
#include "ProjectPanel.h"

/*
	Toolbar is a UI component that owns buttons and visual state and that calls back to Mainframe
*/
class ProjectPanel;

class Toolbar : public wxEvtHandler
{
	public:

		enum ToolID {
				ID_StartStop = wxID_HIGHEST + 100,
				ID_ToggleTable,
				ID_ToggleGraph,
				ID_Reset,
				ID_CollectNow,
				ID_Export,
				//ID_Theme,
				ID_Config,

			// menu-only IDs (not toolbar tools)
			ID_Config_ToggleTheme,
		};

		Toolbar(wxFrame* parent);

		//callbacks
		std::function<void()> onStartStop;
		std::function<void()> onReset;
		std::function<void()> onCollectNow;
		std::function<void()> onExport;
		std::function<void()> onConfig;
		std::function<void()> onToggleTheme;

		//visual state
		void bindEvents(wxFrame* parent);
		void setRunning(bool running);
		void applyTheme(Theme theme);

		//show/hide control
		void setCurrentProject(ProjectPanel* panel) { m_currentProject = panel; }

		//getters
		wxToolBar* get() { return m_toolbar; }
		wxStaticText* getTimerDisplay() { return m_timerDisplay; } //getter for timer

		//private helpers
                void startTimer();
                void stopTimer();
		void onTimer(wxTimerEvent& evt);

                void toggleTable();
                void toggleGraph();
		void toggleCollectNow();

	private:

		wxToolBar* m_toolbar = nullptr;
		wxStaticText* m_timerDisplay = nullptr;
		bool m_isRunning = false;

		//timer state
		wxTimer m_timer;
		int m_elapsedSeconds = 0;

		ProjectPanel* m_currentProject = nullptr;

};
