//
// Created by T14s on 10/22/25.
//
#include "libserialport.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include "SerialComm.h"
#include <string.h>
#include <iostream>
#include <format>
#include <chrono>
#include <thread>





SerialComm::SerialComm()
{
	//the first thing we need to do is determine which, if any, ports are available. We want to query this
	//and store a list of ports names that we can open a line of communication with.
	scanPorts();
}

SerialComm::~SerialComm()
{
	sp_free_port_list(port_list);
	sp_free_config(default_config);
	//cleanPort();

}

void SerialComm::scanPorts()
{
        /* Call sp_list_ports() to get the ports. The port_list
         * pointer will be updated to refer to the array created. */
        enum sp_return result = sp_list_ports(&port_list);

	//if there was some kind of error listing ports, we will throw an exception:
	if (result != SP_OK)
	{
		throw std::runtime_error("Problem scanning ports");
	}
}

/*This function takes no parameters, but returns a read-only reference to the internal vector containing
 * the names of all available serial ports.
 */
const std::vector<std::string>& SerialComm::getPortNames() const
{
	return port_name_list;
}

struct sp_port ** SerialComm::getPortList() const
{
	return port_list;
}

void SerialComm::handshake(std::string portname)
{

	/* We'll allow a 1 second timeout for send and receive. */
	unsigned int timeout = 1500;

	//first things first, assuming that this function gets called
	//multiple times, we need to avoid the situation where a port
	//has already been opened up, so let's assume that each call
	//wants us to clean things up first before re-attempting a
	//new handshake
	
	if (port_status == PORT_OPEN)
	{
		std::cout << "port was open, closing:\n";
		sp_close(port);
		sp_flush(port, static_cast<sp_buffer>(SP_BUF_INPUT | SP_BUF_OUTPUT));

	}



	//the first thing to do is to attempt to open a port with a string name sent as a parameter:

	/* Call sp_get_port_by_name() to find the port. The port
         * pointer will be updated to refer to the port found.
	 *
	 * NOTE: since sp_get_port_by_name() takes a first
	 * parameter as a c-style string, we convert the incoming
	 * string portname during the function call*/
        check(sp_get_port_by_name(portname.c_str(), &port));

	/*Here, we open the port, using the port struct defined
	 * above in the previous step.*/
	std::cout << "Opening port\n";
	check(sp_open(port, SP_MODE_READ_WRITE));
	port_status = PORT_OPEN;

	/*Next, we instantiate the values for our internal variable
	 * default_config, so that we can load this config into the
	 * newly opened port*/

	//allocate memory for the default_config:
       	check(sp_new_config(&default_config));

	//setting the port parameters for default_config
	check(sp_set_config_baudrate(default_config, 9600));
	check(sp_set_config_bits(default_config, 8));
        check(sp_set_config_parity(default_config, SP_PARITY_NONE));
        check(sp_set_config_stopbits(default_config, 1));
        check(sp_set_config_flowcontrol(default_config, SP_FLOWCONTROL_NONE));

	/*Now we load the port config into the port we are using to
	 * communicate*/
	check(sp_set_config(port, default_config));

	std::string data = "Hello";

	//convert the string to char*
	const char* data_char = toCharPtr(data);

	int size = strlen(data_char);

	/* On success, sp_blocking_write() and sp_blocking_read()
	 * return the number of bytes sent/received before the
         * timeout expired. We'll store that result here. */
	int result;

	/* Send data. */
        printf("Sending '%s' (%d bytes) on port %s.\n",
	data_char, size, sp_get_port_name(port));

	result = check(sp_blocking_write(port, data_char, size, timeout));

	/* Check whether we sent all of the data. */
	if (result == size)
	  	printf("Sent %d bytes successfully.\n", size);
	else
	       	printf("Timed out, %d/%d bytes sent.\n", result, size);

	/*Now, we want to receive information from the connected
	 * device.
	 */

	/*Allocate memory for the C-string to store the expected
	 * returned value
	 */
	char *buf = (char*)malloc(size + 1);

	/*Next, we need to read from the port using a function call
	 *provided by the library*/
	//TODO REMOVE THIS AND BROADCAST TO STATUS
	printf("Receiving %d bytes on port %s.\n", size, sp_get_port_name(port));

     	result = check(sp_blocking_read(port, buf, size, timeout));

	/*sp_blocking_read() returns the number of bytes that are actually
	* read. We can use this to compare with the number of expected bytes
	* to determine if there was a problem.
	*/
	if (result == size)
	{
		//TODO
		//REMOVE THIS AND BROADCAST TO STATUS
	        printf("Received %d bytes successfully.\n", size);
	}
	else
	{
		//TODO
		//REMOVE THIS AND BROADCAST TO STATUS
		printf("Timed out, %d/%d bytes received.\n", result, size);
	}



	/* Next, we check to see if the actual data sent matches the data
	 * we sent to the port. The connected device should read the data
	 * sent, and return the exact same data back to the host device.
	 */
	buf[result] = '\0';
		printf("Recieved '%s'.\n", buf);

	free(buf);

	cleanPort();


}

void SerialComm::cleanPort()
{
		check(sp_close(port));
		port_status = PORT_CLOSED;
		sp_free_port(port);
		sp_flush(port, static_cast<sp_buffer>(SP_BUF_INPUT | SP_BUF_OUTPUT));

}

/* Helper function for error handling. */
int SerialComm::check(enum sp_return result)
{
        /* For this example we'll just exit on any error by calling abort(). */
	std::string error_message = sp_last_error_message();
        switch (result) {
        case SP_ERR_ARG:
                throw std::runtime_error("Error: Invalid argument fuck you.\n");
        case SP_ERR_FAIL:
		throw std::runtime_error(std::format("Error: Failed: {}", error_message));
	case SP_ERR_SUPP:
                throw std::runtime_error("Error: Not supported.\n");
	case SP_ERR_MEM:
                throw std::runtime_error("Error: Couldn't allocate memory.\n");
        case SP_OK:
        default:
                return result;
        }
}

/*Helper function to convert strings into c-strings:
 */
const char* SerialComm::toCharPtr(const std::string& s)
{
	return s.c_str();
}
