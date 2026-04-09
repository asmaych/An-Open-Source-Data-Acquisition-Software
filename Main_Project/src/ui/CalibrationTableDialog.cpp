#include "CalibrationTableDialog.h"
#include "CalibrationCompareDialog.h"
#include <wx/wx.h>
#include <wx/grid.h>

CalibrationTableDialog::CalibrationTableDialog(
		wxWindow* parent,
		SensorManager* sensorManager,
		long sensor_index,
		DatabaseManager* db,
    		int projectId,
    		int sensorId,
    		int pin)
	: wxDialog(
			parent,
			wxID_ANY,
			"Enter Calibration reference points",
			wxDefaultPosition,
			wxSize(400,300)),
	m_sensorManager(sensorManager),
	m_sensor_index(sensor_index),
	m_db(db),
      	m_projectId(projectId),
      	m_sensorId(sensorId),
      	m_pin(pin)
{
	//first, make the main sizer
	auto* mainSizer = new wxBoxSizer(wxVERTICAL);

	//checkbox that lets the user save this calibration to the global sensor template, making it accessible to all projects and only
	//shows when the sensor exists in the db
	if(m_sensorId >= 0){
    		m_saveToTemplate = new wxCheckBox(this, wxID_ANY, "Save/update this calibration to sensor template");
    		mainSizer->Add(m_saveToTemplate, 0, wxALL, 10);
	}

	//now define the grid bounds
	m_grid = new wxGrid(this, wxID_ANY);
	m_grid->CreateGrid(5,2);		//initially, 5 rows by 2 columns
	m_grid->SetColLabelValue(0, "Raw");
	m_grid->SetColLabelValue(1, "Mapped");

	m_grid->SetDefaultEditor(new wxGridCellFloatEditor());
	m_grid->SetColFormatFloat(0,6,2);
	m_grid->SetColFormatFloat(1,6,2);
	m_grid->EnableEditing(true);

	m_grid -> SetMinSize(wxSize(-1, 150));
	mainSizer->Add(m_grid, 1, wxEXPAND | wxALL, 10);

	//now add some controls
	wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
	//add them to the main sizer
	mainSizer->Add(buttonSizer, 1, wxALIGN_CENTER | wxBOTTOM, 10);

	SetSizer(mainSizer);
	Layout();

	//now bind the ok button to an event handler
	Bind(wxEVT_BUTTON, &CalibrationTableDialog::onOkPressed, this, wxID_OK);
	m_grid->Bind(wxEVT_GRID_CELL_CHANGED, &CalibrationTableDialog::onCellEdited, this);
	m_grid->Bind(wxEVT_KEY_DOWN, &CalibrationTableDialog::onEnterPressed, this);

	//lock in the reference to local scope
	m_calibrationPoints = m_sensorManager -> getSensorCalibration(m_sensor_index);

	//only load points if calibration exists, nullptr means uncalibrated
	if(m_calibrationPoints && !m_calibrationPoints -> empty())
		loadCalibrationPoints(*m_calibrationPoints);
}

void CalibrationTableDialog::getCalibrationPoints()
{
	//initialize the vector m_table
	m_table = std::make_unique<std::vector<CalibrationPoint>>();

	//get the number of rows that we need to convert into pairs
	int rows = m_grid->GetNumberRows();

	for (int i = 0; i < rows; i++)
	{
		//get the raw value
		wxString rawVal = m_grid->GetCellValue(i,0);
		//get the mapped value
		wxString mappedVal = m_grid->GetCellValue(i,1);

		//now convert to numeric types and create a calibrationpoint pair
		double x,y;
		if (rawVal.ToDouble(&x) && mappedVal.ToDouble(&y))
		{
			//push the values to the vector m_table
			m_table->push_back({x,y});
		}
	}
}

void CalibrationTableDialog::loadCalibrationPoints(const std::vector<CalibrationPoint> & table) {

	if(!m_calibrationPoints || table.empty())
        	return;

    	//the grid starts with 5 rows by default. If the calibration has more points than rows, append the extra rows first without 
	//this, writing to row index >= GetNumberRows() triggers a wxGrid assertion failure
    	int needed = static_cast<int>(table.size());
    	int existing = m_grid->GetNumberRows();

    	if(needed > existing)
        	m_grid -> AppendRows(needed - existing);

    	//now fill each row safely, the grid is guaranteed to have enough rows for all the calibration points
    	for(int i = 0; i < needed; ++i){
        	m_grid -> SetCellValue(i, 0, wxString::Format("%.6f", table[i].raw));
        	m_grid -> SetCellValue(i, 1, wxString::Format("%.6f", table[i].mapped));
	}
}

void CalibrationTableDialog::onCellEdited(wxGridEvent& evt) {
	//get the index of the row that was just edited
	int row = evt.GetRow();

	//get the index of the last row currently existing
	int lastrow = m_grid->GetNumberRows()-1;

	//now check to see if this cell has any data. If it does, we expand, otherwise we do nothing
	bool changed = false;

	for (int col = 0; col <2; col++) {
		if (!wxIsEmpty(m_grid->GetCellValue(row, col))) {
			changed = true;
			break;
		}
	}

	if (changed) {
		m_grid->AppendRows(1);
	}

	//let any other handlers do their job as well
	evt.Skip();


}

void CalibrationTableDialog::onEnterPressed(wxKeyEvent& evt) {
	if (evt.GetKeyCode() == WXK_RETURN) {
		//get the current row in focus by the user
		int row = m_grid->GetGridCursorRow();

		//get the last row currently in the table
		int lastrow = m_grid->GetNumberRows()-1;

		//if they are the same, then we append a new row
		if (row == lastrow) {
			m_grid->AppendRows(1);
		}
	}

	evt.Skip();
}

void CalibrationTableDialog::onOkPressed(wxCommandEvent& evt)
{
	//vectorize the table:
	getCalibrationPoints();

	if (m_table->empty())
	{
		wxMessageBox("Table cannot be empty");
		return;
	}
	else if (m_table->size() < 2)
	{
		wxMessageBox("Please enter at least two numerical calibration pairs");
		return;
	}

	//make a copy of the points before moving m_table into the Interpolator meaning before becoming null
    	std::vector<CalibrationPoint> pointsCopy = *m_table;

    	//create the interpolator and assign it to the sensor
    	auto interpolator = std::make_unique<Interpolator>(std::move(m_table));
    	m_sensorManager->setCalibration(m_sensor_index, std::move(interpolator));

    	//save calibration to DB if we have a valid project context.
    	if(m_db && m_projectId >= 0 && m_sensorId >= 0 && m_pin >= 0){
        	bool saved = m_db -> saveProjectCalibration(m_projectId, m_sensorId, m_pin, "table", pointsCopy);

        	if(saved)
            		std::cout << "Calibration saved to DB for sensor_id = " << m_sensorId << " pin = " << m_pin << "\n";
        	else
            		std::cerr << "Calibration DB save failed\n";
    	}

	//save to global sensor template if the checkbox is checked
	if(m_saveToTemplate && m_saveToTemplate -> GetValue() && m_db && m_sensorId >= 0){
    		if(m_db -> hasGlobalCalibration(m_sensorId)){
        		//load the existing global calibration so we can show it alongside the new one in the comparison dialog
        		std::string existingType;
        		std::vector<CalibrationPoint> existingPoints;
        		m_db -> loadGlobalCalibration(m_sensorId, existingType, existingPoints);

        		//show the side-by-side comparison dialog so the user can see exactly what changes before overwriting
        		CalibrationCompareDialog compareDlg(this, existingPoints, pointsCopy);

        		if(compareDlg.ShowModal() == wxID_OK){
            			//user confirmed the overwrite
            			m_db -> saveGlobalCalibration(m_sensorId, "table", pointsCopy);
            			std::cout << "Global calibration overwritten for sensor_id = " << m_sensorId << "\n";
        		}
        		else{
            			//user cancelled means global untouched, project save already done
            			std::cout << "Global calibration overwrite cancelled\n";
        		}
    		}
    		else{
        		//no existing global calibration means save directly, no dialog needed
        		m_db -> saveGlobalCalibration(m_sensorId, "table", pointsCopy);
        		std::cout << "Global calibration saved for sensor_id = " << m_sensorId << "\n";
    		}
	}

	this -> EndModal(wxID_OK);
}
