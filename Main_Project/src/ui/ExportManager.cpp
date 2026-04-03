#include "ExportManager.h"
#include "data/DataTableWindow.h"
#include "data/GraphWindow.h"
#include <wx/filedlg.h>
#include <fstream>

/**
 * @brief constructs an ExportManager bound to a parent window and sensor list
 */
ExportManager::ExportManager(wxWindow* parent, const std::vector<std::unique_ptr<Sensor>>& sensors)
    	      : m_parent(parent), m_sensors(sensors)
{
}


/**
 * @brief shows the export options dialog(graph, collectNow, collectContinuous) and performs the selected exports
 */
void ExportManager::exportSessions(const std::shared_ptr<Run>& currentRun, const std::vector<std::shared_ptr<Run>>& allRuns, DataTableWindow* tableWindow, GraphWindow* graphWindow)
{
    	//allow export if we have either a current run or historical runs cause when a project is loaded, 
	//m_currentRun is null but allRuns has data.
    	bool hasCurrent = (currentRun != nullptr);
    	bool hasAny = (!allRuns.empty());

    	if(!hasCurrent && !hasAny)
        	return;

    	//use currentRun if active, otherwise go back to the last historical run
    	std::shared_ptr<Run> runToExport = currentRun ? currentRun : allRuns.back();

    	//abort early if the selected run contains no frame data
    	bool hasData = false;
    	for(const auto& frame : runToExport -> getFrames()){
        	if(!frame.empty()){
            		hasData = true;
            		break;
        	}
    	}

    	if(!hasData){
        	wxMessageBox("No data available to export!", "Error", wxOK | wxICON_ERROR);
        	return;
    	}

    	// ====== Export options dialog =======
    	//lets the user choose which outputs to generate (collectCOntinuous, graph...) before jumping to a save path
    	wxDialog dlg(m_parent, wxID_ANY, "Export options", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    	//show each checkbox based on whether that window exists
    	wxCheckBox* cbContinuous = new wxCheckBox(&dlg, wxID_ANY, "Collect Continuous");
    	wxCheckBox* cbCollectNow = new wxCheckBox(&dlg, wxID_ANY, "Collect Now");
    	wxCheckBox* cbGraph = new wxCheckBox(&dlg, wxID_ANY, "Graph");

    	cbContinuous -> SetValue(currentRun != nullptr);
    	cbCollectNow -> SetValue(tableWindow != nullptr);
    	cbGraph -> SetValue(graphWindow != nullptr);

    	sizer -> Add(cbContinuous, 0, wxALL, 5);
    	sizer -> Add(cbCollectNow, 0, wxALL, 5);
    	sizer -> Add(cbGraph, 0, wxALL, 5);

    	wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    	btnSizer -> AddButton(new wxButton(&dlg, wxID_OK));
    	btnSizer -> AddButton(new wxButton(&dlg, wxID_CANCEL));
    	btnSizer -> Realize();

    	sizer -> Add(btnSizer, 0, wxEXPAND | wxALL, 10);
    	dlg.SetSizerAndFit(sizer);

    	if(dlg.ShowModal() != wxID_OK)
        	return;

    	//in case collect-now was selected but the table was never created
    	if(cbCollectNow -> IsChecked() && !tableWindow){
        	wxMessageBox("Collect now table wasn't created!", "Export Error", wxOK | wxICON_WARNING);
        	return;
    	}

    	// ====== Ask for save path ======
    	wxString path = askSaveFile(m_parent);
    	if(path.IsEmpty())
        	return;

    	//remove extension so we can append our own per-output suffixes
    	path = path.BeforeLast('.');

    	// ====== Do the exports =======
    	bool didExport = false;

    	if(cbContinuous -> IsChecked() && runToExport){
        	exportRun(runToExport, path + "_collect_continuous.csv");
       		didExport = true;
    	}

    	if(cbCollectNow -> IsChecked() && tableWindow){
	        exportTable(tableWindow, path + "_collect_now.csv");
        	didExport = true;
    	}

    	if(cbGraph -> IsChecked() && graphWindow){
        	graphWindow -> exportImage(path + "_graph.png");
        	didExport = true;
    	}

    	if(!didExport){
        	wxMessageBox("Nothing was selected to export.", "Export", wxOK | wxICON_WARNING);
        	return;
    	}

    	wxMessageBox("Export Complete!", "Info", wxOK | wxICON_INFORMATION);
}


/**
 * @brief opens a save file dialog and returns the chosen path
 */
wxString ExportManager::askSaveFile(wxWindow* parent)
{
    	wxFileDialog dlg(parent, "Export data", "", "", "CSV files (*.csv)|*.csv|Text files (*.txt)|*.txt",
									wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    	if(dlg.ShowModal() == wxID_CANCEL)
        	return "";

    	wxString path = dlg.GetPath();

    	//append default extension if the user did not type one which is csv
    	if(!path.EndsWith(".csv") && !path.EndsWith(".txt"))
        	path += ".csv";

    	return path;
}


/**
 * @brief writes a continuous run to a CSV file
 */
void ExportManager::exportRun(const std::shared_ptr<Run>& run, const wxString& path)
{
    	if(!run)
		return;

    	auto& times  = run -> getTimes();
    	auto& frames = run -> getFrames();

    	if(times.empty() || frames.empty())
		return;

    	std::ofstream file(path.ToStdString());

	if(!file.is_open())
		return;

    	//header row: Time + one column per sensor
    	file << "Time";
    	for(size_t s = 0; s < frames[0].size(); ++s)
        	file << "," << m_sensors[s] -> getName();
    	file << "\n";

    	//data rows: one row per frame
    	for(size_t i = 0; i < times.size(); ++i){
        	file << times[i];
        	for(auto val : frames[i])
            		file << "," << val;
        	file << "\n";
    	}

    	file.close();
}


/**
 * @brief writes the collect-now table to a CSV file
 */
void ExportManager::exportTable(DataTableWindow* table, const wxString& path)
{
    	if(!table || m_sensors.empty())
		return;

    	std::ofstream file(path.ToStdString());
    	if(!file.is_open())
		return;

    	const auto& times  = table -> getTimes();
    	const auto& values = table -> getValues();

    	if(times.empty())
		return;

    	//header row
    	file << "Time";
    	for(size_t s = 0; s < values.size(); ++s)
        	file << "," << m_sensors[s] -> getName();
    	file << "\n";

    	//data rows
    	for(size_t row = 0; row < times.size(); ++row){
        	file << times[row];
        	for(size_t s = 0; s < values.size(); ++s)
            		file << "," << values[s][row];
        	file << "\n";
    	}

    	file.close();
}


/**
 * @brief writes graph curve data to a CSV file
 */
void ExportManager::exportGraph(GraphWindow* graph, const wxString& path)
{
    	if(!graph)
		return;

    	const auto& curves = graph -> getCurves();
    	if(curves.empty())
		return;

    	//append to existing file if it already exists from exportRun/exportTable
    	std::ofstream file(path.ToStdString(), std::ios::app);
    	if(!file.is_open())
		return;

    	//header row: one column per curve label
    	file << "Time";
    	for(const auto& curve : curves)
        	file << "," << curve.label;
    	file << "\n";

    	//find the shortest curve length to avoid out-of-bounds reads(crashes)
    	size_t points = curves[0].x.size();
    	for(const auto& curve : curves)
        	points = std::min(points, curve.x.size());

    	//data rows: time from first curve, values from all curves
    	for(size_t j = 0; j < points; ++j){
        	file << curves[0].x[j];
        	for(const auto& curve : curves){
            		double v = (j < curve.y.size()) ? curve.y[j] : 0.0;
            		file << "," << v;
        	}
        	file << "\n";
    	}

    	file.close();
}
