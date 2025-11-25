#include <vector>
#include "SensorConfigDialog.h"
#include "AddSensorDialog.h"
#include "SerialComm.h"
#include "SensorManager.h"
#include <wx/listctrl.h>
#include <wx/wx.h>

SensorConfigDialog::SensorConfigDialog(wxWindow* parent,
					const wxString& title,
					SerialComm* serialComm,
					SensorManager* sensorManager,
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

	//main sizer for the button group and the sensor list
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	//create the button for add sensor:
	wxButton* add_sensor = new wxButton(this, wxID_ANY, "Add Sensor");
	//add the button to the buttonSizer:
	buttonSizer->Add(add_sensor,0, wxALL | wxALL, 10);

	//create the button for remove sensor:
	wxButton* remove_sensor = new wxButton(this, wxID_ANY, "Remove Selected");
	//add the button to the buttonsizer:
	buttonSizer->Add(remove_sensor, 0, wxALL | wxALL, 10);

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
	
	add_sensor->Bind(wxEVT_BUTTON, &SensorConfigDialog::onAddSensorPressed, this);
	remove_sensor->Bind(wxEVT_BUTTON, &SensorConfigDialog::onRemoveSensorPressed, this);
	

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
	if (m_serialComm->handshakeresult == false)
	{
		wxMessageBox("Please connect with a microcontroller first!");
		return;
	}

	//next check to see if we are at the limit for sensors allowed
	if (m_list->GetItemCount() == 10)
	{
		wxMessageBox("You are already at the maximum number of sensors");
		return;
	}


	//simply launch the AddSensorDialog and pass along the
	//SensorManager pointer to the new dialog
	AddSensorDialog sensor_adder(this, "Configure New Sensor", m_sensorManager);

	//implicitly set the dialog to modal, and see
	//if it returns wxID_OK to indicate a successful 
	//operation. If so, we refresh the table of sensors
	if (sensor_adder.ShowModal() == wxID_OK)
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
	long item = m_list->GetNextItem(-1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED);

	//if the user has not selected a sensor from the list
	//then item will reflect that in its index, so we 
	//prompt the user to select one and return the function
	if (item == -1)
	{
		wxMessageBox("No sensor selected.", "Error");
		return;
	}

	//otherwise, we proceed to remove the coresponding sensor
	
	//get the sensors name corresponding to the selection
	wxString sensorName = m_list->GetItemText(item);

	//now use sensorManager to remove it after confirming
	if (wxMessageBox(
			"Remove selected sensor?",
			"Confirm",
			wxYES_NO | wxICON_QUESTION) ==wxYES)
	{
		//remove it
		m_sensorManager->removeSensor(sensorName.ToStdString());

	}

	//finally, repopulate the table after removing the sensor
	populateTable();
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
