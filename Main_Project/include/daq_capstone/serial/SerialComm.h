//
// Created by T14s on 10/22/25.
//
#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <string>
#include <vector>
#include "Sensor.h"
#include <memory>
#include <set>

//explicitly declaring the library for libserialport:
#include <libserialport.h>

enum PORT_STATUS { PORT_OPEN, PORT_CLOSED };

class SerialComm
{
	public:
		SerialComm();
		~SerialComm();

		//keeping it as an atomic  method allows us to re-scan at any time we want.
		void scanPorts();

		//getter method to return a reference to the vector port_name_list
		const std::vector<std::string>& getPortNames() const;
		
		//getter method to return the null-terminated array of actual ports
		struct sp_port ** getPortList() const;

		//this function sends a "ping\n" and returns true if it receieves
		//"pong\n" in return.
		bool handshake(std::string portname);

		/* This is a helper function that closes the comm port
		 * and frees the memory associated with all the data-
		 * structures used in the port operation
		 */
		void cleanPort();


		/*This is a helper function that uses an enum provided
		 * by libserialport to handle different error codes that
		 * may occur as a result of using library function calls
		 */
		int check(enum sp_return result);

		void addSensor(const std::string& sensorName, int pin);
		void removeSensor(const std::string& sensorName);
		void adjustPollingRate(int rate);
		void reset();
		bool writeString(const std::string& str);

		void readDataFrame(std::vector<std::unique_ptr<Sensor>>& sensors, 
                                   std::string* rawFrame = nullptr);

		void flush();

		bool handshakeresult = false;






	private:
        	/* A pointer to a null-terminated array of pointers to
         	 * struct sp_port, which will contain the ports found.
		 */
        	struct sp_port **port_list;

		//declare the string vector we need to store the values in plaintext.
		std::vector<std::string> port_name_list;

		/* This is a struct that represents a set of serial
		 * port parameters. This struct can be directly applied
		 * to an open port to change the parameters.
		 *
		 * This particular instance will represent the default communication
		 * parameters used for communication with arduino devices.
		 */
		 struct sp_port_config *default_config;

		/* This struct represents a "port" object that can be used by
		 * libserialport library. It is being declared as a private variable
		 * here because we wish to use it across multiple class methods.
		 *
		 * We will only open communication via a single port, and this is
		 * that port that will always be used, unconditionally.
		 */
		struct sp_port *port = nullptr;

		/* Adding a variable to keep track of the port status*/
		PORT_STATUS port_status = PORT_CLOSED;

		/*tracker for the port used by this instance
		 */
		std::string m_portName;

		/*static set of portnames to avoid port conflicts between instances
		 * of SerialComm
		 */
		static std::set<std::string> g_ports_in_use;
};

#endif
