#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <string>
#include <vector>

//explicitly declaring the library for libserialport:
#include <libserialport.h>

class SerialComm
{
	public:
		//take no parameters. the first thing we need to do is generate a list of ports for the user:
		SerialComm();

		//deconstruct the object
		~SerialComm();
		//helper function that populates a string vector with the names of all available serial ports

		//keeping it as an atomic  method allows us to re-scan at any time we want. 
		void scanPorts();

		//declaring a getter method that returns the port list as a vector
		//we set it as a const and simply return a reference to the original. 
		//This ensures data integrity and efficiency (avoids copies)
		const std::vector<std::string>& getPorts() const;

		/* This function opens a specified port for communication using
		 * serial communication. The parameters are set to a default
		 * value, and a packet of data is sent to the connected device.
		 *
		 * if the device returns the same packet of data, then the 
		 * handshake is a success, as successful communication has
		 * occurred. Otherwise, the operation was not a success,
		 * and we will need to re-attempt the handshake somehow.
		 *
		 * Further functionalities and customizations TBD*/
		bool handshake(std::string portname);

		/* This is a helper function that closes the comm port
		 * and frees the memory associated with all the data-
		 * structures used in the port operation
		 */
		bool cleanPort();


		/*This is a helper function that uses an enum provided
		 * by libserialport to handle different error codes that
		 * may occur as a result of using library function calls
		 */
		int check(enum sp_return result);






	private:
        	/* A pointer to a null-terminated array of pointers to
         	 * struct sp_port, which will contain the ports found.*/
        	struct sp_port **port_list;

		//declare the string vector we need to store the values in plaintext.  
		std::vector<std::string> port_name_list;

		/* This is a struct that represents a set of serial
		 * port parameters. This struct can be directly applied
		 * to an open port to change the parameters. 
		 *
		 * This particular instance will represent the default communication
		 * parameters used for communication with arduino devices.*/
		 struct sp_port_config *default_config;

		/* This struct represents a "port" object that can be used by 
		 * libserialport library. It is being declared as a private variable
		 * here because we wish to use it across multiple class methods.
		 *
		 * We will only open communication via a single port, and this is 
		 * that port that will always be used, unconditionally.*/
		struct sp_port *port;

		/* Helper function that takes a std::string as an input
		 * and returns a char* string as output. This is necessary
		 * because the library call sp_blocking_write() requires
		 * that the data being written is in the style of a c-string
		 *
		 * This function gets the input string passed by reference,
		 * so that there is no need to allocate new memory. The 
		 * output of the function will be a temporary copy of the 
		 * converted string. 
		 */
		const char* toCharPtr(const std::string& s);

};

#endif

