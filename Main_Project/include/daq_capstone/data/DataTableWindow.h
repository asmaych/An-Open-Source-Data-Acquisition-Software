#pragma once
#include <wx/wx.h>
#include <wx/grid.h> // Grid widget used for displaying and editing tabular data
#include <memory>
#include "data/DataSession.h"
#include "data/Run.h"
#include "ui/Theme.h"
#include "ui/ProjectPanel.h"

class ProjectPanel;
class Run;
enum class Theme;

/**
 * @brief GUI object that displays values in a table
 *
 * DataTableWindow is a single table window that displays collected sensor values in a table.
 * After an experiment, we can show all values collected for a sensor in a structured table (wxGrid)
 * where each value appears in a separate row. It supports dynamic update (append/remove columns as needed)
*/
class DataTableWindow : public wxPanel
{
	public:
		DataTableWindow(wxWindow* parent, const std::vector<std::shared_ptr<DataSession>>& sessions,
				std::shared_ptr<Run> run = nullptr);

		//update table with latest values
		void updateTable();

		//set which sensors to display (dynamic)
		void setSelectedSessions(const std::vector<std::shared_ptr<DataSession>>& sessions);

		//append a single value to a specific column
		void appendRow(const std::vector<double>& rowValues);

		//change the table theme
		void applyTheme(Theme theme);

		std::shared_ptr<Run> getAssociatedRun() const;

		const std::vector<double>& getTimes() const { return m_times; }
		const std::vector<std::vector<double>>& getValues() const { return  m_values; }
		wxGrid* getGrid() const { return m_grid; }

	private:
		std::shared_ptr<Run> m_associatedRun;
		std::vector<std::shared_ptr<DataSession>> m_sessions; //sessions currently displayed
		std::vector<double> m_times;
                std::vector<std::vector<double>> m_values;
		wxGrid* m_grid; //Grid widget to display the table
		wxButton* m_collect_button;
};
