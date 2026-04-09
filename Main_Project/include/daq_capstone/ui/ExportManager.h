#pragma once
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <memory>
#include <vector>
#include "data/Run.h"
#include "sensor/Sensor.h"

class DataTableWindow;
class GraphWindow;

/**
 * @brief handles all data export operations for a project (csv files and graph png), it doesn't own any data, it just uses whats 
 *        being passed in.
 */
class ExportManager
{
    	public:
        	/**
         	  * @brief constructs an ExportManager linked to a parent window and sensor list
         	  *
         	  * @param parent     Parent wxWindow
         	  * @param sensors    Reference to the project's sensor vector, and used for for column headers
         	*/
        	ExportManager(wxWindow* parent, const std::vector<std::unique_ptr<Sensor>>& sensors);

        	/**
         	  * @brief shows the export options dialog and performs the selected exports
         	  *
         	  * User can choose which outputs to export (continuous run CSV, collect-now CSV, graph PNG)
		  * Asks for a save path and writes each selected output to the chosen directory
         	  *
         	  * @param currentRun    The active or most recent run, this might be nullptr if only historical runs exist
         	  * @param allRuns       All runs stored in this project session
         	  * @param tableWindow   The collect-now table window which can also be null
         	  * @param graphWindow   The graph window
         	*/
        	void exportSessions(const std::shared_ptr<Run>& currentRun,const std::vector<std::shared_ptr<Run>>& allRuns,
            			    DataTableWindow* tableWindow,GraphWindow* graphWindow);

        	/**
         	  * @brief opens a save file dialog and returns the chosen path according to the user, returns an empty string if 
		  * the user cancels, and appends .csv if the user did not provide a .csv .txt extension
         	  *
         	  * @param parent  Parent window for the dialog
         	  * @return        Full file path chosen by the user, or empty string
         	*/
        	static wxString askSaveFile(wxWindow* parent);

    	private:
        	wxWindow* m_parent = nullptr;

        	//reference to the project's sensor list, used for CSV column headers
        	const std::vector<std::unique_ptr<Sensor>>& m_sensors;

        	/**
         	  * @brief writes a continuous run to a CSV file
         	  *
         	  * Columns: Time, then one column per sensor in m_sensors order
         	  * Rows correspond to individual frames recorded during the run
         	  *
         	  * @param run   Shared pointer to the run that we will export
         	  * @param path  Full output file path
         	*/
        	void exportRun(const std::shared_ptr<Run>& run, const wxString& path);

        	/**
         	  * @brief writes the collect-now table to a CSV file
         	  *
         	  * Columns: Time, then one column per sensor
		  * Rows correspond to individual on-demand collect events
         	  *
         	  * @param table  Pointer to the DataTableWindow to export
         	  * @param path   Again full output file path
         	*/
        	void exportTable(DataTableWindow* table, const wxString& path);

        	/**
         	  * @brief writes graph curve data to a CSV file
                  *
         	  * @param graph  Pointer to the GraphWindow to export
         	  * @param path   Full output file path
         	*/
        	void exportGraph(GraphWindow* graph, const wxString& path);
};
