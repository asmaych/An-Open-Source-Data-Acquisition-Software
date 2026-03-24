#pragma once
#include <string>
#include <vector>
#include "Sensor.h"
#include <memory>
#include <set>

//explicitly declaring the library for libserialport:
#include <libserialport.h>

enum PORT_STATUS { PORT_OPEN, PORT_CLOSED };

/**
 * @brief This is the class used for all serial-related operations in the application.
 *
 * This is the Class that is used solely and exclusively by the application for any and all operations involving
 * serial read, serial write, or any serial configuration. It is the single point of access that all other entities
 * should use for any serial operations.
 *
 * This class specifically, handles configuring, opening, and closing serial ports, and exposes public methods for
 * sending commands to the microcontroller to modify its runtime behavior.
 */
class SerialComm
{
	public:
		/**
		* @brief Conducts initial scan of available connections, and creates a default port configuration.
		*
		* When run, this constructor will create a port configuration struct defined by the libserialport
		* library. This struct contains the parameters under which a serial port will be opened. By default,
		* the baud rate is set to 115,200.
		*
		* The function SerialComm::scanPorts will also be called, and will load an initial list of available serial
		* connections
		*/
		SerialComm();

		/**
		* @brief Necessary for freeing allocated libserialport objects
		*
		* When run, this destructor will free the memory that was allocated for structs defined by the
		* libserialport library to hold port-related information. If these resources are not freed, the
		* port channel will be left open, and no other entities will be able to access it.
		*/
		~SerialComm();

		//keeping it as an atomic  method allows us to re-scan at any time we want.
		/**
		* @brief atomic method for updating a list of serial connections
		*
		* When called, this method uses the libserialport sp_list_ports() to populate the private
		* class member port_list with available connections. Port list is a null terminated array
		* of structs defined by libserialport that represent available port connections.
		*
		*	@param [out] port_list This private member attribute will be updated on each scan.
		*/
		void scanPorts();

		//getter method to return the null-terminated array of actual ports
		/**
		* @brief getter method to return a null-terminated array of port structs
		*
		 * @return [sp_port ** port_list]
		*/
		struct sp_port ** getPortList() const;

		//this function sends a "ping\n" and returns true if it receieves
		//"pong\n" in return.
		/**
		* @brief Performs a handshake with a device over a serial port.
		*
		* This function attempts to establish communication with a device connected
		* to the specified serial port by sending a "ping\n" message and waiting
		* for a "pong\n" response.
		*
		* The function will:
		* - Open the specified serial port
		* - Send a "ping\n" message
		* - Wait (with timeout) for a response
		* - Read incoming data until a newline character is encountered
		* - Compare the received message against the expected "pong"
		*
		* If the expected response is received, the handshake is considered successful.
		*
		* @param [in] portname The name of the serial port to connect to.
		* @param [out] port The private class member pointer to a port struct defined by libserialport
		*
		* @return true  If the device responds with "pong"
		* @return false If the response is incorrect or a timeout/error occurs
		*
		* @throws std::runtime_error If the specified port is already in use by another instance
		*
		* @note The port is marked as "in use" upon successful handshake to prevent
		*       multiple instances from accessing the same port.
		*
		* @warning This function performs blocking I/O operations and may take up to
		*          the timeout duration to return.
		*/
		bool handshake(std::string portname);


		/*This is a helper function that uses an enum provided
		 * by libserialport to handle different error codes that
		 * may occur as a result of using library function calls
		 */
		/**
		 * @brief this is a helper function that is used to catch and handle errors in libserialport function calls
		 *
		 * All libserialport function calls return an enum value, based on the result of the function. For instance, if
		 * there are no errors, then the enum value returned is SP_OK. If there is an error of some kind, the enum value
		 * will reflect that.
		 *
		 * This function is used to wrap around libserialport function calls, and it specifically looks for any error
		 * related enum return values, and then handles them - if they occur - inside a switch block. If the enum value
		 * is SP_OK, the function simply returns, and the execution will continue after the function call as normal.
		 */
		static int check(enum sp_return result);

		/**
		 * @brief This function is used to add a sensor to the microcontroller's polling routine
		 *
		 * This function will confirm if a serial port connection is opened, and if so, will generate and send a command
		 * over the opened port to the microcontroller. This command modifies the runtime behavior of the microcontroller
		 * by adding a new pin from which readings are polled.
		 *
		 * @param [in] sensorName This is the name corresponding to the sensor we wish to add. It is needed here because
		 * the microcontroller maintains a vector of sensor names, whose order is the same as the actual vector of
		 * Sensor objects owned by each instance of ProjectPanel. Each name is associated with a pin from which the
		 * Microcontroller polls on a scheduled routine. By maintaining this ordered vector, dataframes can be generated
		 * in an indexed fashion, and this makes the parsing of dataframes much easier.
		 *
		 * @param [in] pin This is the pin from which the microcontroller will begin to read after receiving the add
		 * command.
		 *
		 * @note Currently, the only entity that should be interacting with this method is the SensorManager class.
		 * Any and all sensor-related additions, removals, or modifications, should be done through SensorManager.
		 *
		 * @warning This function writes a blind command to the serial output buffer. There is no built-in parameter
		 * validation, or any safety logic. Undefined behavior may result if this method is invoked without care. For
		 * instance, a pin may be specified that does not physically exist on the microcontroller, or a name may be
		 * specified two times, with different pins.
		 *
		 */
		void addSensor(const std::string& sensorName, int pin);

		/**
		 * @brief This function is used to remove a sensor from the microcontroller's polling routine
		 *
		 * This function will confirm if a serial port connection is opened, and if so, will generate and send a command
		 * over the opened port to the microcontroller. This command modifies the runtime behavior of the microcontroller
		 * by removing the specified name from the vector, and its associated pin, from the microcontroller memory.
		 *
		 * @param [in] sensorName This is the name corresponding to the sensor we wish to Remove. When the microcontroller
		 * receives this command, it will remove the entry corresponding to sensorName from the internal vector of names.
		 * Additionally, it will clear the mapped entry between sensorName, and its associated pin, from which readings
		 * are polled.
		 *
		 * @note Currently, the only entity that should be interacting with this method is the SensorManager class.
		 * Any and all sensor-related additions, removals, or modifications, should be done through SensorManager.
		 *
		 * @warning This function writes a blind command to the serial output buffer. There is no built-in parameter
		 * validation, or any safety logic. Undefined behavior may result if this method is invoked without care.
		 *
		 */
		void removeSensor(const std::string& sensorName);

		/**
		 * @brief This function sends a command to the microcontroller that adjusts its sensor polling rate
		 *
		 * @param rate This is the number that specifies to the microcontroller how many milliseconds to delay
		 */
		void adjustPollingRate(float rate) const;

		/**
		 * @brief This function sends a command to the microcontroller that hard-resets the microcontroller
		 *
		 * This function is necessary, because otherwise, when a ProjectPanel instance is closed, the microcontroller
		 * will continue to stream data according to its last runtime configuration. This constant stream of data will
		 * pollute the incoming serial buffer, making future handshake attempts fail.
		 *
		 * By invoking this function, the microcontroller clears all runtime-configured variables and data, and begins
		 * awaiting another handshake command.
		 */
		void reset();

		/**
		 *
		 * @param str uuuuuuhhh
		 * @return hmmmmmmm
		 */
		bool writeString(const std::string& str);

		/**
		 * @brief This function reads a single dataframe, parses it, and assigns readings to Sensor objects
		 *
		 * When this function is called, it reads from the serial input buffer exactly one dataframe, which is
		 * demarcated by a newline character. The dataframe coming from the microcontroller contains comma separated
		 * values, which are indexed according the order of Sensor objects stored in the vector in ProjectPanel.
		 *
		 * The dataframe is parsed, and each Sensor is assigned its corresponding reading.
		 *
		 * This method is designed to run at a rapid rate on a background polling thread. It simply reads a dataframe,
		 * parses it, and assigns values to each Sensor that has been configured in a project. It does not care what is
		 * done with those readings - it simply updates them.
		 *
		 * @param sensors This is a reference to an std::vector of smart pointers to Sensor objects owned by ProjectPanel.
		 * It is used to load parsed readings into each Sensor, using its Sensor::setReading() method.
		 */
		void readDataFrame(std::vector<std::unique_ptr<Sensor>>& sensors);

		/**
		 * @brief This function flushes the input and output serial buffers for the currently opened serial port
		 *
		 *
		 */
		void flush();

		/**
		 * @brief This is used by outside classes to check whether SerialComm is connected to a microcontroller
		 */
		bool handshakeresult = false;






	private:
		/**
		 * @brief this is a helper function used to help wrap memory cleaning operations
		 *
		 * This function flushes the incoming and outgoing serial buffers, closes the serial connection to the serial
		 * port pointed to by port, and frees the port name for use by other instances of SerialComm by removing the
		 * name of the port from the global attribute set g_ports_in_use
		 *
		 * @param [out] port The private class member pointer to a port struct defined by libserialport
		 * @param [out] g_ports_in_use The global set of names used by SerialComm instances to check if a port is in use
		 *
		 * @note As a built-in safety measure, this function includes a check before anything, that immediately returns
		 * if there is not opened port to clean up. This shouldn't be an issue in general, but it doesn't hurt to have
		 * redundancy.
		 *
		 */
		void cleanPort();
		/**
		* @brief This is used to hold pointers to the port structs generated by libserialport.sp_list_ports()
		*/
		struct sp_port **m_port_list;

		/**
		 * @brief This is used to store a more accessable form of port identifiers for use in GUI applications
		 */
		std::vector<std::string> m_port_name_list;

		/**
		 * @brief This struct is for a default configuration for serial connections. It is configured in the constructor
		 */
		struct sp_port_config * m_default_config;

		/* This struct represents a "port" object that can be used by
		 * libserialport library. It is being declared as a private variable
		 * here because we wish to use it across multiple class methods.
		 *
		 * We will only open communication via a single port, and this is
		 * that port that will always be used, unconditionally.
		 */
		/**
		 * @brief This is used across the class to point to the single serial connection that is open for a ProjectPanel
		 */
		struct sp_port * m_port = nullptr;

		/**
		 * @brief This is used only internally to make sure that operations aren't attempted on a closed port
		 */
		PORT_STATUS m_port_status = PORT_CLOSED;

		/**
		 * @brief Used to add and remove the name of the port from the global set g_ports_in_use
		 */
		std::string m_portName;

		/**
		 * @brief set of port names to avoid port conflicts between instances of SerialComm
		 */
		static std::set<std::string> g_ports_in_use;
};