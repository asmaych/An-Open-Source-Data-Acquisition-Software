#pragma once
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <functional>
#include <wx/timer.h>
#include "Theme.h"
#include "ProjectPanel.h"

class ProjectPanel;

/**
 * @brief GUI object that encapsulates all Toolbar controls into one class.
 *
 * The Toolbar currently includes controls for:
 *	-	Start/Stop Run
 *	-	Table Display Toggle
 *	-	Graph Display Toggle
 *	-	Reset Run
 *	-	On Demand Toggle
 *	-	Export Data
 *	-	ProjectConfig Launcher
 *
 *	Callbacks are used extensively in this class to affect the in-focus ProjectPanel via MainFrame
 */
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

		//syncs all toolbar visual state to match the given project, called by mainFRame whenever the active tab changes
		void syncFromProject(ProjectPanel* panel);

	private:

		wxToolBar* m_toolbar = nullptr;
		wxStaticText* m_timerDisplay = nullptr;
		bool m_isRunning = false;

		//timer state
		wxTimer m_timer;

		ProjectPanel* m_currentProject = nullptr;

		Theme m_theme;
};
