#include "controllers/SessionController.h"
#include "data/DataSession.h"
#include "sensor/Sensor.h"
#include "data/Run.h"
#include "ui/ProjectPanel.h"

SessionController::SessionController()
	//we initialize the atomic boolean m_running to false so that when the controller is created, it's not running
	: m_running(false)
{

}

SessionController::SessionController(SerialComm* serial,
                                     std::vector<std::shared_ptr<Run>>* runs,
                                     ProjectPanel* panel,
				     std::vector<std::unique_ptr<Sensor>>* sensors)
    : m_serial(serial),
      m_runs(runs),
      m_panel(panel),
      m_sensors(sensors),
      m_running(false)
{

}


SessionController::~SessionController()
{
	//stop the background thread if it is running
	stop();
}

bool SessionController::start()
{
	//we expected the m_running to be false, since we can only start when its not running
	bool expected = false;

	//atomic compare_change: checks if m_running == expected(false), we set m_running to true else false
	if(m_running.compare_exchange_strong(expected, true)){
		m_thread = std::thread([this](){
			while(m_running.load()){

				if(m_serial && m_sensors){
					std::string rawFrame;
					//read from serial and update sensor readings
					m_serial -> readDataFrame(*m_sensors, &rawFrame);

					//send raw frame to a project panel's live window and run storage
					if(!rawFrame.empty() && m_panel){
						auto frameCopy = rawFrame;  // important: copy data

						m_panel->CallAfter([this, frameCopy]() {
							m_panel->onNewDataFrame(frameCopy);
						});
					}
				}
				//read sensor data
				std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(m_sampleRate.load()));
			}
		});
		return true; // we transitioned to running
	}
	return false; //already running
}

bool SessionController::stop()
{
	//to stop the controller must be running
	bool expected = true;
	if(m_running.compare_exchange_strong(expected, false)){
		if(m_thread.joinable()) m_thread.join();
		return true;
	}
	return false; //already stopped
}

void SessionController::setInterval(const float rate)
{
	m_sampleRate.store(rate);
	std::cout << "Setting rate to " << rate << std::endl;
}

bool SessionController::toggle()
{
	//flip state automatically 
	if(isRunning())
		return stop();

	else
		return start();
	
//RIght now since we are using one microprocessor we wont face a problem with toggle, but if in the future we jump to receiving
// real time data from multiple sensors on more than one device = multi-threads, the the compare and fetch could interfere.
}

void SessionController::reset(std::vector<std::unique_ptr<DataSession>>& sessions)
{
	/* This function takes a vector of unique_ptr<DataSession> by reference, it loops over every single Datasession
	   and for non-null sessions, it calls clear to reset/empty all of them.
	   Note: if other thrads are reading/writing sessions, the caller must protect that
	*/
	for(auto& session: sessions) {
		if(session){
			session -> clear();
		}
	}
}

bool SessionController::isRunning() const
{
	//returns the current value of the atomic variable m_running, load is used in this case cause it's an atomic boolean
	return m_running.load();
} 
