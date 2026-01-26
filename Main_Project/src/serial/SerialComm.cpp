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
#include "Sensor.h"
#include <memory>
#include <set>

//initialize the global set of port names
std::set<std::string> SerialComm::g_ports_in_use;

SerialComm::SerialComm()
{
        //the first thing we need to do is determine which, if any, ports are available. We want to query this
        //and store a list of ports names that we can open a line of communication with.
        scanPorts();

        //set the default config for port communication:

        //allocate memory for the default_config:
        check(sp_new_config(&default_config));

        //setting the port parameters for default_config
        check(sp_set_config_baudrate(default_config, 9600));
        check(sp_set_config_bits(default_config, 8));
        check(sp_set_config_parity(default_config, SP_PARITY_NONE));
        check(sp_set_config_stopbits(default_config, 1));
        check(sp_set_config_flowcontrol(default_config, SP_FLOWCONTROL_NONE));

}

SerialComm::~SerialComm()
{
        //we can run these first commands, because they run automatically on constructor
        sp_free_port_list(port_list);
        sp_free_config(default_config);

        //but for this one, we only need it if the pointer port has been assigned
        if (port != nullptr)
                cleanPort();

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

bool SerialComm::handshake(std::string portname)
{
	/* \brief	This function takes the name of a port and
	 * 		attempts to send a "ping\n" packet to the device
	 * 		connected at the other side. Upon successful
	 * 		receipt of "pong\n" from the device, the function
	 * 		will return true.
	 *
	 * 		If the read operation for the port times out, or
	 * 		something other than "pong\n" is receieved, then
	 * 		the function will return false.
	 */

	//---------------------------------------------------------------------------------------------------
	//MAKE SURE THAT THE PORT IS NOT ALREADY IN USE BY ANOTHER SERIALCOMM OBJECT
	//---------------------------------------------------------------------------------------------------
	if (g_ports_in_use.count(portname))
	{
		throw std::runtime_error("Port already in use by another project!");
	}
	//otherwise, proceed with the handshake

	//capture the incoming name as a c style string
	//This is necessary because libserialport requires
	//the use of char* instead of std::string
	const char* c_portname = portname.c_str();

	//set timeout to one second
	unsigned int timout = 1000;

	//set the message to send to the microcontroller device
	const char* send = "ping\n";
	//calculate its size to avoid redundant calls later
	int size = strlen(send);

	//declare a var to store the result of read operations
	int result;

	//---------------------------------------------------------------------------------------------------
	//ATTEMPT TO OPEN PORT WITH GIVEN NAME PARAMETER
	//---------------------------------------------------------------------------------------------------
	
	//get the port struct object using libserialport function
	check(sp_get_port_by_name(c_portname, &port));

	//try to open the port for reading and writing
	check(sp_open(port, SP_MODE_READ_WRITE));
	//if there are no problems and we get to this point, set the port flag to OPEN
	port_status = PORT_OPEN;

	//add the port name to the global list so no other instances try to use it
	g_ports_in_use.insert(portname);
	m_portName = portname;

	//load the default configuration into the port
	check(sp_set_config(port, default_config));

	//---------------------------------------------------------------------------------------------------
	//CLEAR ANYTHING THAT MAY HAVE BEEN IN THE INCOMING BUFFER
	//---------------------------------------------------------------------------------------------------
	sp_flush(port, static_cast<sp_buffer>(SP_BUF_INPUT));

	//---------------------------------------------------------------------------------------------------
	//TRY TO SEND THE "ping\n" PACKET
	//---------------------------------------------------------------------------------------------------
	
	//result will store the number of bytes written
	result = check(sp_blocking_write(port, send, size, timout));
	
	//quick check to ensure all data was sent:
	if (result == strlen(send))
	{
		printf("Sent %d bytes successfully\n", size);
	}
	else
	{
		printf("Error, %d/%d bytes sent", result, size);
	}

	//---------------------------------------------------------------------------------------------------
	//TRY TO READ THE INCOMING "Pong\n" PACKET
	//---------------------------------------------------------------------------------------------------
	
	//to accomplish the reading of a single message while avoiding malloc,
	//we read bytes from the input stream until we reach a newline

	//make a buffer of char to read into
	char buffer[32];
	int pos = 0;

	while (pos < sizeof(buffer)-1)
	{
		//for each index of the char buffer, read a byte into
		//that index
		int n = sp_blocking_read(port, &buffer[pos], 1, 1000);

		//break if the read ever returns an error
		if (n <= 0)
		{
			break;
		}

		//if we reach a newline, also break
		if (buffer[pos] == '\n')
		{
			break;
		}

		//finally, increment the pos for the buffer reader
		pos++;
	}

	//null-terminate at last position
	buffer[pos] = '\0';

	//trim the buffer input
	buffer[strcspn(buffer, "\r\n")] = '\0';

	printf("Received line: %s\n", buffer);

	//---------------------------------------------------------------------------------------------------
	//TRY TO COMPARE THE BUFFER READ WITH EXPECTED "pong\n"
	//---------------------------------------------------------------------------------------------------

	if (strcmp("pong", buffer) == 0)
	{
		printf("Success, expected: \"pong\", received: %s", buffer);
		this->handshakeresult = true;
	}
	else
	{
		printf("Error, expected: \"pong\", received: %s", buffer);
		this->handshakeresult = false;
	}

	return handshakeresult;
}

void SerialComm::flush()
{
        if (port_status == PORT_OPEN)
        {
                sp_flush(port, SP_BUF_INPUT);
                sp_flush(port, SP_BUF_OUTPUT);
        }
}

void SerialComm::adjustPollingRate(int rate)
{
	/* \brief 	This function takes an int as a parameter and simply sends
	 * 		it to the connected arduino controller to control how many
	 * 		times per second a reading is generated.
	 */

	//first, check to make sure the port is open
	if(port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string adjust_command = "adjust," + std::to_string(rate) + "\n";

	//send the command
	(void) sp_blocking_write(port, adjust_command.c_str(), adjust_command.size(), 1000);

}

void SerialComm::removeSensor(const std::string& sensorName)
{
	/* \brief 	This function takes a string as a parameter and uses it to
	 * 		prompt the connected microcontroller to stope reading and
	 * 		sending data for it.
	 */

	//first check to make sure the port is open
	if (port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string remove_command = "remove," + sensorName + "\n";

	//send the command
	(void) sp_blocking_write(port, remove_command.c_str(), remove_command.size(), 1000);
}

void SerialComm::addSensor(const std::string& sensorName, int pin)
{
	/* \brief 	This function takes a name and a pin number as parameters
	 * 		in order to send a command to the microcontroller to
	 * 		start generating data for this sensor.
	 */

	std::cout << "oh boyyy we are trying to add with the arduinoo\n";
	//first make sure the port is open
	if (port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string add_command = "add," + sensorName + "," + std::to_string(pin) + "\n";

	//send the command
	(void) check(sp_blocking_write(port,add_command.c_str(), add_command.size(), 1000));

}

void SerialComm::readDataFrame(std::vector<std::unique_ptr<Sensor>>& sensors, std::string* rawFrame)
{
	 /* \brief       This function takes a reference to the vector of sensors
         *              owned by Project, reads a data-frame from the connected 
         *              microcontroller, and parses it into individual indexed
         *              sensor readings. The readings are assigned to each
         *              sensor in the vector, and the function call terminates.
         *
         *              If there is no data available from the microcontroller,
         *              the function terminates without doing anything.
         *
         *              This function is designed to run on a continous
         *              background thread - updating each sensor reading in the
         *              project with new values as they are transmitted.
         */


        //quick check to make sure the port is opened for communication
	if (port_status == PORT_CLOSED)
        	throw std::runtime_error("Error: Attempt to poll data from closed port");

	//---------------------------------------------------------------------------------------------------
        //READ A DATAFRAME FROM THE MICROCONTROLLER
        //---------------------------------------------------------------------------------------------------

        //make a buffer to store input
        //max sensors = 10 (for now)  -> "r1,r2,r3,...,r10" <- dataframe structure
        //
        //9 commas, plus 10 readings of 0-4096 means a maximum of 49 characters in a dataframe
        //
        //let's just allocate 64 for now, and come back to this later if it bites us
        
	char buffer[64];

    	// read as much data as possible in a single blocking call
    	// - buffer size is limited to prevent overflow
   	// - timeout is 100 ms
    	int n = sp_blocking_read(port, buffer, sizeof(buffer) - 1, 100);

 	// If no bytes were read, exit early
    	if (n <= 0)
        	return;

    	// null-terminate the received data so it can be treated as a C-string
   	buffer[n] = '\0';

    	// remove newline or carriage return characters from the frame
    	buffer[strcspn(buffer, "\r\n")] = '\0';

    	// optionally return the raw frame string to the caller if requested
    	if (rawFrame)
        	*rawFrame = buffer;
	//std::cout << buffer << std::endl;

    	// ---------------- PARSE FRAME ----------------
    	// we expect a frame format of: "value1,value2,value3,..."

    	// pointer used to iterate through the buffer character by character
    	char* ptr = buffer;

    	// pointer marking the start of the current numeric token
    	char* start = buffer;

    	// index used to map parsed values to sensor objects
	// note that it is of type size_t to match with the return type of sensors.size()
	std::size_t index = 0;

    	// walk through the buffer until end-of-string or all sensors are filled
    	for (; *ptr != '\0' && index < sensors.size(); ++ptr)
    	{
        	// comma indicates the end of one numeric value (value of one sensor)
        	if (*ptr == ',')
        	{
            		// replace comma with null terminator to isolate the number
            		*ptr = '\0';

            		// convert ASCII number to integer and store in sensor
            		sensors[index]->setReading(atoi(start));

            		// jump to the next sensor
            		index++;

           		// move start pointer to the character after the comma
            		start = ptr + 1;
        	}
    	}

    	// handle the final value after the last comma (or the only value)
    	if (index < sensors.size() && *start != '\0')
    	{
        	// convert and store the final sensor reading
        	sensors[index]->setReading(atoi(start));
    	}

}

void SerialComm::reset()
{
	/* \brief 	This function takes no parameters, and simply sends a single
	 * 		command to the microntroller. This command causes a complete
	 * 		reset of the microcontroller device.
	 *
	 * 		This is only intended to be run when the ProjectPanel is done
	 * 		interacting with the physical hardware - for instance, when it
	 * 		is close - and we want reset the initial state of the device so
	 * 		that other projects can use it.
	 */

	std::cout << "oh boyyy we are resettingg the arduinooo\n";
	//first make sure the port is open
	if (port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string reset_command = "reset\n";

	//send the command
	(void) check(sp_blocking_write(port,reset_command.c_str(), reset_command.size(), 1000));
}

bool SerialComm::writeString(const std::string& str)
{
    if (!port) return false;       // port is your opened serial port
    std::string s = str + "\n";    // append newline if your ESP32 expects it
    int written = sp_blocking_write(port, s.c_str(), s.size(), 1000);
    return written == (int)s.size();
}

void SerialComm::cleanPort()
{
                sp_flush(port, static_cast<sp_buffer>(SP_BUF_INPUT | SP_BUF_OUTPUT));
                check(sp_close(port));
		sp_free_port(port);

		//free the name of the port from the global list
		g_ports_in_use.erase(m_portName);


		//reset member attributes, because this instance might yet be used for 
		//a new port with a different configuration.
		port = nullptr;
                port_status = PORT_CLOSED;
		m_portName.clear();
}

/* Helper function for error handling. */
int SerialComm::check(enum sp_return result)
{
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


