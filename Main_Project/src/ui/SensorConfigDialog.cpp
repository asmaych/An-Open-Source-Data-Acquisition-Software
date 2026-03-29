#include <vector>
#include "SensorConfigDialog.h"
#include "AddSensorDialog.h"
#include "CalibrateSensorDialog.h"
#include "SerialComm.h"
#include "SensorManager.h"
#include <wx/listctrl.h>
#include <wx/wx.h>

SensorConfigDialog::SensorConfigDialog(wxWindow* parent,
					const wxString& title,
					SerialComm* serialComm,
					SensorManager* sensorManager,
					DatabaseManager* Database,
					std::vector<std::unique_ptr<Sensor>>& sensors)
	: wxDialog(
			parent,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(500,400),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	m_serialComm(serialComm),
	m_sensorManager(sensorManager),
	m_DB(Database),
	m_sensors(sensors)
		
{

	//-----------------------------------------------------------------
	//ADD CONTROLS
	//-----------------------------------------------------------------
	
	//this design will use two sizers. One is for the buttons for
	//"add" and "remove" sensor. These will be contained at the top, and
	//will be sized horizontally. The second is to contain the button 
	//sizer, and size it appropriately with a central list of current
	//sensors below in the dialog. The sizer for the buttons will be
	//arranged vertically with the sensor list.
	
	//sizer for the button grouping
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	//main sizer for the button group, sensor list, and sample rate config
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//create the button for add sensor:
	wxButton* add_sensor = new wxButton(this, wxID_ANY, "Add Sensor");
	//add the button to the buttonSizer:
	buttonSizer->Add(add_sensor,0, wxALL | wxALL, 10);

	//create the button for remove sensor:
	wxButton* remove_sensor = new wxButton(this, wxID_ANY, "Remove Selected");
	//add the button to the buttonsizer:
	buttonSizer->Add(remove_sensor, 0, wxALL | wxALL, 10);

	//create the button for calibrate Sensor:
	wxButton* calibrate_sensor = new wxButton(this, wxID_ANY, "Calibrate Sensor");
	//add the button to the buttonsizer:
	buttonSizer->Add(calibrate_sensor, 0, wxALL | wxALL, 10);

	//load from database button
	wxButton* loadFromDB = new wxButton(this, wxID_ANY, "Load from Database");
	buttonSizer -> Add(loadFromDB, 0, wxALL, 10);

	//create a button for selecting the sensors that will be used in a  project x
	//wxButton* addToProject = new wxButton(this, wxID_ANY, "Add Selected to Project");
	//add the button to the buttonsizer:
	//buttonSizer -> Add(addToProject, 0, wxALL, 10);

	//add the button grouping to the mainsizer
	mainSizer->Add(buttonSizer, 0, wxALIGN_LEFT);

	//configure the list of sensors
	m_list = new wxListCtrl(this,
				wxID_ANY,
				wxDefaultPosition,
				wxDefaultSize,
				wxLC_REPORT | wxLC_SINGLE_SEL);

	m_list->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT,150);
	m_list->InsertColumn(1, "Pin", wxLIST_FORMAT_LEFT, 120);

	//add the table to the mainSizer
	mainSizer->Add(m_list, 1, wxEXPAND | wxALL, 10);

	//adjust all the fitting of all controls:
	SetSizerAndFit(mainSizer);

	//---------------------------------------------------------------------------------------------------
	//BINDING THE CONTROLS TO EVENT HANDLERS
	//---------------------------------------------------------------------------------------------------
	
	add_sensor -> Bind(wxEVT_BUTTON, &SensorConfigDialog::onAddSensorPressed, this);
	remove_sensor -> Bind(wxEVT_BUTTON, &SensorConfigDialog::onRemoveSensorPressed, this);
	calibrate_sensor -> Bind(wxEVT_BUTTON, &SensorConfigDialog::onCalibratePressed, this);
	loadFromDB -> Bind(wxEVT_BUTTON, &SensorConfigDialog::onLoadFromDatabasePressed, this);
	//addToProject -> Bind(wxEVT_BUTTON, &SensorConfigDialog::onAddToProject, this);

	//now populate the list using helper function
	populateTable();
}

void SensorConfigDialog::onAddSensorPressed(wxCommandEvent& evt)
{
	/* \brief	This function handles the event that the button for 
	 * 		adding a new sensor is pressed in the dialog interface.
	 *
	 * 		The only thing necessary is to stack allocate an instance
	 * 		of the AddSensorDialog, passing along the pointer to the
	 * 		sensorManager owned by the parent Project. Any modification
	 * 		to the vector of sensors is made by SensorManager, and not
	 * 		this or any other dialog.
	 */

	//add a check to see if we have connected with a device yet
	if (!m_serialComm->handshakeresult)
	{
		wxMessageBox("Please connect with a microcontroller first!");
		return;
	}

	ProjectPanel* project = dynamic_cast<ProjectPanel*>(GetParent());
	int projectID = (project && project -> shouldSaveProject()) ? project -> getProjectID() : -1;

	//simply launch the AddSensorDialog and pass along the
	//SensorManager pointer to the new dialog
	AddSensorDialog sensor_add_dialog(this, "Configure New Sensor", m_sensorManager, m_DB, projectID);

	//implicitly set the dialog to modal, and see
	//if it returns wxID_OK to indicate a successful 
	//operation. If so, we refresh the table of sensors
	if (sensor_add_dialog.ShowModal() == wxID_OK)
	{
		//refresh the table of sensors
		populateTable();
	}
}

void SensorConfigDialog::onRemoveSensorPressed(wxCommandEvent& evt)
{
	/* \brief 	This function handles the event that the button for
	 * 		removing a sensor is pressed.
	 *
	 * 		The list below allows for a selection by a user, and
	 * 		the function of the remove button is to remove that 
	 * 		selection. Some validation is added to make sure that
	 * 		clicking remove without a selection is handled, and
	 * 		an additional prompt is sent to the user to confirm
	 * 		the deletion.
	 *
	 * 		The deletion is performed entirely by the m_sensorManager,
	 * 		which is a raw pointer to the central SensorManager object 
	 * 		in the parent Project.
	 *
	 */
	//find the sensor we want to remove
	long selected_sensor = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	//if the user has not selected a sensor from the list
	//then item will reflect that in its index, so we 
	//prompt the user to select one and return the function
	if (selected_sensor == -1)
	{
		wxMessageBox("No sensor selected.", "Error");
		return;
	}

	//otherwise, we proceed to remove the coresponding sensor
	
	//get the sensors name corresponding to the selection
	wxString sensor_name = m_list->GetItemText(selected_sensor);

	//get the pin from column 1 of the list
    	wxListItem pinItem;
    	pinItem.SetId(selected_sensor);
    	pinItem.SetColumn(1);
    	pinItem.SetMask(wxLIST_MASK_TEXT);
    	m_list -> GetItem(pinItem);
    	int pin = wxAtoi(pinItem.GetText());

	//now use sensorManager to remove it after confirming
	if (wxMessageBox(
			"Remove selected sensor?",
			"Confirm",
			wxYES_NO | wxICON_QUESTION) ==wxYES)
	{
		//remove it
		m_sensorManager->removeSensor(sensor_name.ToStdString(), pin);

	}

	//finally, repopulate the table after removing the sensor
	populateTable();
}

void SensorConfigDialog::onCalibratePressed(wxCommandEvent& evt)
{
	/* \brief	This function handles the event that the button for
	 * 		calibrating a sensor is pressed.
	 *
	 * 		The list of sensors allows the user to make a selection,
	 * 		and the function of the calibrate button is to open up
	 * 		another dialog in which the user can enter datapoints to
	 * 		configure the sensor calibration via table interpolation.
	 *
	 * 		If the button is pressed without a valid selection, this
	 * 		function will not do anything.
	 */

	//find the sensor entry corresponding to the user selection:
	long selected_sensor = m_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	//check to see if the user actually has made a selection.
	//If they have not, prompt them to do so, and do nothing
	if (selected_sensor == -1)
	{
		wxMessageBox("No sensor selected.", "Error");
		return;
	}

	//otherwise, we proceed to open a calibration dialog for the selected sensor
	
	//get the name of the sensor that corresponds to the selection
	wxString sensor_name = m_list->GetItemText(selected_sensor);


	//now call the calibration dialog for further steps
	CalibrateSensorDialog sensor_calibrator(this, "Sensor Calibration", m_sensorManager, selected_sensor);
	//make the dialog visible and modal:
	sensor_calibrator.ShowModal();

	//nothing should visibly change for our list of sensors upon
	//successful calibration, so we don't need to do anything else
}

void SensorConfigDialog::populateTable()
{
	/* \brief 	This function has a single purpose: to refresh the 
	 * 		displayed table to accurately reflect the current
	 * 		contents of the m_sensors vector.
	 *
	 * 		It begins by deleting all items, and then repopulating
	 * 		the table based on the contents of m_sensors, and this
	 * 		is easier than manually removing or adding elements,
	 * 		because we can avoid an unecessary search in the case
	 * 		of a removal. Additionally, we ensure that the order
	 * 		displayed in the table always corresponds to the order
	 * 		of Sensors in m_sensors.
	 */

	//first, make sure the table is clear
	m_list->DeleteAllItems();

	//now populate it with the contents of m_sensors
	for (size_t i = 0; i < m_sensors.size(); i++)
	{
		//for every sensor s in the vector of sensors
		const Sensor* s = m_sensors[i].get();
		
		//insert an entry in the table, where the first 
		//field is the sensor's name
		long idx = m_list->InsertItem(i, s->getName());

		//set the same item's second field to be the
		//sensor's pin number
		m_list->SetItem(idx, 1, std::to_string(s->getPin()));

		//now store index of the vector that this item
		//came from, so that when we need to delete it
		//later, we can retrieve the index directly
		m_list->SetItemData(idx, i);
	}
}

void SensorConfigDialog::onLoadFromDatabasePressed(wxCommandEvent& evt)
{
	/* \brief	This function runs when the user presses the "Load From
			Database" button.

			- It reads sensor template names from the db
			- It shows them in a checklist dialog
			- It lets the user choose which ones to use
			- It asks the user to assign a pin for each one
			- It creates REAL Sensor objects for the project
			- It does update the UI table
	*/

	//creates a vector to store sensor names loaded from the db
	std::vector<std::string> names;

	//call loadSensorNames function to read the sensors table from the db and fills the vector with all stored names
	m_DB -> loadSensorTemplates(names);

	//if db is empty, let the user know
	if(names.empty()){
		wxMessageBox("No sensors stored in the database!");
		return;
	}

	//we create a dialog window that shows all sensor names where the user can check the ones they wanna add
	SensorSelectionDialog selectionDialog(this, names);

	//show dialog modally (blocks until user presses ok or cancel)
	if(selectionDialog.ShowModal() != wxID_OK)
		return; //user cancelled

	//get selected sensor indexes
	auto selectedIndexes = selectionDialog.getSelectedIndexes();

	//get the project id from the parent panel so we can link loaded sensors to this project in project_sensors
	ProjectPanel* project = dynamic_cast<ProjectPanel*>(GetParent());
	int projectID = (project && project -> shouldSaveProject()) ? project -> getProjectID() : -1;

	//for each sensor template
	for(int index : selectedIndexes){
		//get the name of the selected template
		std::string selectedName = names[index];

		//ask the user to assign a pin
		int pin = askUserForPin(selectedName);

		//if user cancelled or invalid pin, skip
		if(pin < 0 || pin > 20){
			wxMessageBox("Pin must be between 1 and 19");
			continue;
		}

		if(m_sensorManager -> pinExists(pin)){
			wxMessageBox("Pin already used!");
			continue;
		}

		//add the sensor to the project in memory.
        	if(m_sensorManager -> addSensor(std::make_unique<Sensor>(selectedName, pin))){
           	 	//link this sensor to the project in project_sensors
            		if(projectID >= 0){
                		int sensorID = m_DB->getSensorID(selectedName);
                		if(sensorID >= 0){
                    			m_DB->saveProjectSensor(projectID, sensorID, pin);
                    			std::cout << "Loaded sensor '" << selectedName << "' linked to project " << projectID << "\n";
                		}
            		}
		}
	}
	//refresh the ui table, so the user can see the newly added sensors
	populateTable();
}


int SensorConfigDialog::askUserForPin(const std::string& name){
	//create a dialog that asks for pin number
	wxTextEntryDialog dialog(this, "Enter pin number for sensor: " + name, "Assign Pin");

	//show the dialog and wait for user input
	if(dialog.ShowModal() != wxID_OK)
		return -1;

	//variable to store the converted pin number
	long pin;

	//convert users text to a number, if it fails that means false
	if(!dialog.GetValue().ToLong(&pin)){
		wxMessageBox("Invalid pin!");
		return -1;
	}

	return static_cast<int>(pin);
}
