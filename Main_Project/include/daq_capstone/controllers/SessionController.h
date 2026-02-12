#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include "serial/SerialComm.h"
#include "ui/ProjectPanel.h"
#include "data/Run.h"

/* SessionController is a class that is responsible for start/stop/reset lifecycle handling.
   It doesn't itself poll the serial port, projectPanel still polls.
   Instead it manages the "running" state, reset logic (cleaning sessions), and exposes simple control API used by toolbar buttons.
*/
class ProjectPanel;
class SerialComm;
class DataSession;
class DataCollector;
class Sensor;

class SessionController {

	public:
		SessionController();
		SessionController(SerialComm* serial, std::vector<std::shared_ptr<Run>>* runs,
					ProjectPanel* panel, std::vector<std::unique_ptr<Sensor>>* sensors);

		~SessionController();

		//Start the experiment (set running as true). returns true if started.
		bool start();

		//Stop the experiment (set running false), returns true if stopped.
		bool stop();

		//toggle start/stop 
		bool toggle();

		//Reset sessions: clear all stored DataSessions passed in.
		void reset(std::vector<std::unique_ptr<DataSession>>& sessions);

		//Query running state
		bool isRunning() const;

	private:
		SerialComm* m_serial = nullptr;
    		std::vector<std::shared_ptr<Run>>* m_runs = nullptr;
     		ProjectPanel* m_panel = nullptr;
		std::vector<std::unique_ptr<Sensor>>* m_sensors = nullptr;
		std::atomic<bool> m_running;
		std::thread m_thread;
		std::mutex m_mutex; //protects complex transitions if needed
};
