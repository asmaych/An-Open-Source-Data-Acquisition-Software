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


SerialComm::SerialComm()
{
	//the first thing we need to do is determine which, if any, ports are available. We want to query this
	//and store a list of ports names that we can open a line of communication with.
	scanPorts();
}

SerialComm::~SerialComm()
{
	sp_free_port_list(port_list);
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
		throw std::runtime_error("Problem connecting with the port");
	}

	//garbage cleanup and data safety:
	port_name_list.clear();

	//now we append the port names to the string vector. We want this because we want
	//users to be able to access a simple drop-down list of the port names.
	for (int i=0; port_list[i] != NULL; i++)
	{
		//let *port point to one of the pointers to a port struct in the array port_list
		port = port_list[i];

		port_name_list.emplace_back(std::string(sp_get_port_name(port))
				+ std::string(sp_get_port_description(port)));
	}
}

/*This function takes no parameters, but returns a read-only reference to the internal vector containing
 * the names of all available serial ports.
 */
const std::vector<std::string>& SerialComm::getPorts() const
{
	return port_name_list;
}

bool SerialComm::handshake(std::string portname)
{

	try
	{
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
		check(sp_open(port, SP_MODE_READ_WRITE));

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

		std::string data = "Hello World!!";

		//convert the string to char*
		const char* data_char = toCharPtr(data);

		int size = strlen(data_char);

		/* We'll allow a 1 second timeout for send and receive. */
		unsigned int timeout = 1000;

		/* On success, sp_blocking_write() and sp_blocking_read()
	         * return the number of bytes sent/received before the
	         * timeout expired. We'll store that result here. */
	        int result;

		/* Send data. */
	        printf("Sending '%s' (%d bytes) on port %s.\n",
	               data, size, sp_get_port_name(port));

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
	       	printf("Receiving %d bytes on port %s.\n",
	           	size, sp_get_port_name(port));
	     	result = check(sp_blocking_read(port, buf, size, timeout));

		/*sp_blocking_read() returns the number of bytes that are actually
		 * read. We can use this to compare with the number of expected bytes
		 * to determine if there was a problem.
		 */
	      	if (result == size)
			//TODO
			//REMOVE THIS AND BROADCAST TO STATUS
	         	printf("Received %d bytes successfully.\n", size);
	    	else
		{
			//TODO
			//REMOVE THIS AND BROADCAST TO STATUS
			printf("Timed out, %d/%d bytes received.\n", result, size);

			/* Before we throw an exception, we make sure to close the
			 * opened port session, otherwise subsequent attempts to
			 * use this port will be unsuccessful.*/
			cleanPort();

			throw std::runtime_error("");
		}



		/* Next, we check to see if the actual data sent matches the data
		 * we sent to the port. The connected device should read the data
		 * sent, and return the exact same data back to the host device.
		 */
		buf[result] = '\0';
			printf("Recieved '%s'.\n", buf);

		return true;

		}
		catch(const std::runtime_error& e)
		{
			//TODO
			//send status message to status class
			std::cout << "caught exception" << e.what() <<
				"\n Delete this later after implementing status" << std::endl;
			return false;
		}
}

bool SerialComm::cleanPort()
{
	try
	{
		check(sp_close(port));
		sp_free_port(port);
		sp_free_config(default_config);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return false;
	}

}

/* Helper function for error handling. */
int SerialComm::check(enum sp_return result)
{
        /* For this example we'll just exit on any error by calling abort(). */
        char *error_message;
        switch (result) {
        case SP_ERR_ARG:
                printf("Error: Invalid argument fuck you.\n");
                abort();
        case SP_ERR_FAIL:
                error_message = sp_last_error_message();
                printf("Error: Failed: %s\n", error_message);
                sp_free_error_message(error_message);
                abort();
        case SP_ERR_SUPP:
                printf("Error: Not supported.\n");
                abort();
        case SP_ERR_MEM:
                printf("Error: Couldn't allocate memory.\n");
                abort();
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
