#include "CalibrationCompareDialog.h"
#include <wx/stattext.h>
#include <algorithm>

CalibrationCompareDialog::CalibrationCompareDialog(wxWindow* parent, const std::vector<CalibrationPoint>& current, const std::vector<CalibrationPoint>& incoming)
    
			 : wxDialog(parent, wxID_ANY, "Calibration Comparison", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    	//warning message at the top so the user knows what they are doing
    	wxStaticText* warning = new wxStaticText(this, wxID_ANY, "Do you want to overwrite the global calibration that already exists?\n", 
								  wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    	mainSizer->Add(warning, 0, wxALL, 12);

    	//side by side sizer for the two grids
    	wxBoxSizer* gridSizer = new wxBoxSizer(wxHORIZONTAL);

    	//Left grid: current saved calibration
    	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    	wxStaticText* currentLabel = new wxStaticText(this, wxID_ANY, "Current (saved globally)");
    	currentLabel -> SetFont(currentLabel->GetFont().Bold());
    	leftSizer -> Add(currentLabel, 0, wxBOTTOM, 4);

   	m_currentGrid = new wxGrid(this, wxID_ANY);
    	m_currentGrid -> CreateGrid(0, 2);
    	m_currentGrid -> SetColLabelValue(0, "Raw");
    	m_currentGrid -> SetColLabelValue(1, "Mapped");
    	m_currentGrid -> SetRowLabelSize(0);
    	m_currentGrid -> SetColSize(0, 120);
    	m_currentGrid -> SetColSize(1, 120);
    	m_currentGrid -> EnableEditing(false);
    	m_currentGrid -> SetMinSize(wxSize(260, 200));

    	populateGrid(m_currentGrid, current);
    	leftSizer -> Add(m_currentGrid, 1, wxEXPAND);
    	gridSizer -> Add(leftSizer, 1, wxEXPAND | wxRIGHT, 10);

    	//Right grid: new incoming calibration
    	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    	wxStaticText* incomingLabel = new wxStaticText(this, wxID_ANY, "New (yours)");
    	incomingLabel -> SetFont(incomingLabel -> GetFont().Bold());
    	rightSizer -> Add(incomingLabel, 0, wxBOTTOM, 4);

    	m_incomingGrid = new wxGrid(this, wxID_ANY);
    	m_incomingGrid -> CreateGrid(0, 2);
    	m_incomingGrid -> SetColLabelValue(0, "Raw");
    	m_incomingGrid -> SetColLabelValue(1, "Mapped");
    	m_incomingGrid -> SetRowLabelSize(0);
    	m_incomingGrid -> SetColSize(0, 120);
    	m_incomingGrid -> SetColSize(1, 120);
    	m_incomingGrid -> EnableEditing(false);
    	m_incomingGrid -> SetMinSize(wxSize(260, 200));

    	populateGrid(m_incomingGrid, incoming);
    	rightSizer -> Add(m_incomingGrid, 1, wxEXPAND);
    	gridSizer -> Add(rightSizer, 1, wxEXPAND);

    	mainSizer -> Add(gridSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 12);

    	//highlight differing rows after both grids are populated
    	highlightDifferences(current, incoming);

    	//buttons
    	wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

    	wxButton* overwriteBtn = new wxButton(this, wxID_OK, "Overwrite");
    	wxButton* cancelBtn    = new wxButton(this, wxID_CANCEL, "Cancel");

    	btnSizer -> Add(overwriteBtn, 0, wxRIGHT, 10);
    	btnSizer -> Add(cancelBtn,    0);

    	mainSizer -> Add(btnSizer, 0, wxALIGN_CENTER | wxALL, 12);

    	SetSizerAndFit(mainSizer);
    	SetSize(wxSize(600, 500));
    	Centre();
}


void CalibrationCompareDialog::populateGrid(wxGrid* grid, const std::vector<CalibrationPoint>& points)
{
    	if(points.empty())
        	return;

    	grid -> AppendRows(static_cast<int>(points.size()));

    	for(size_t i = 0; i < points.size(); ++i){
        	int row = static_cast<int>(i);
        	grid -> SetCellValue(row, 0,
            	wxString::Format("%.6f", points[i].raw));
        	grid -> SetCellValue(row, 1,
            	wxString::Format("%.6f", points[i].mapped));
    	}
}


void CalibrationCompareDialog::highlightDifferences(const std::vector<CalibrationPoint>& current, const std::vector<CalibrationPoint>& incoming)
{
    	//we compare row by row up to the length of the longer table. Rows that exist in one table but not the other are always
    	//highlighted cause they represent added or removed points
    	size_t maxRows = std::max(current.size(), incoming.size());

    	//get the default font and create a bold version for differing rows
    	wxFont normalFont = m_currentGrid -> GetDefaultCellFont();
    	wxFont boldFont = normalFont;
    	boldFont.SetWeight(wxFONTWEIGHT_BOLD);

    	for(size_t i = 0; i < maxRows; ++i){
        	int row = static_cast<int>(i);
        	bool different = false;

        	if(i >= current.size() || i >= incoming.size()){
            		//row exists in one table but not the other = always different
            		different = true;
        	}
        	else{
            		//exact double comparison = any difference triggers highlight
            		different = (current[i].raw    != incoming[i].raw) || (current[i].mapped != incoming[i].mapped);
        	}

        	if(different){
            		//highlight the row in both grids wherever the row exists
            		if(row < m_currentGrid -> GetNumberRows()){
                		m_currentGrid -> SetCellFont(row, 0, boldFont);
                		m_currentGrid -> SetCellFont(row, 1, boldFont);
            		}
            		if(row < m_incomingGrid -> GetNumberRows()){
                		m_incomingGrid -> SetCellFont(row, 0, boldFont);
                		m_incomingGrid -> SetCellFont(row, 1, boldFont);
            		}
        	}
    	}

    	//force both grids to redraw with the new background colours
    	m_currentGrid -> ForceRefresh();
    	m_incomingGrid -> ForceRefresh();
}
