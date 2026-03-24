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
        check(sp_new_config(&m_default_config));

        //setting the port parameters for default_config
        check(sp_set_config_baudrate(m_default_config, 115200));
        check(sp_set_config_bits(m_default_config, 8));
        check(sp_set_config_parity(m_default_config, SP_PARITY_NONE));
        check(sp_set_config_stopbits(m_default_config, 1));
        check(sp_set_config_flowcontrol(m_default_config, SP_FLOWCONTROL_NONE));

}
SerialComm::~SerialComm()
{
        //we can run these first commands, because they run automatically on constructor
        sp_free_port_list(m_port_list);
        sp_free_config(m_default_config);

        //but for this one, we only need it if the pointer port has been assigned
        if (m_port != nullptr)
                cleanPort();

}

void SerialComm::scanPorts()
{
        /* Call sp_list_ports() to get the ports. The port_list
         * pointer will be updated to refer to the array created. */
		sp_return result = sp_list_ports(&m_port_list);

        //if there was some kind of error listing ports, we will throw an exception:
        if (result != SP_OK)
        {
                throw std::runtime_error("Problem scanning ports");
        }
}

struct sp_port ** SerialComm::getPortList() const
{
        return m_port_list;
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
	check(sp_get_port_by_name(c_portname, &m_port));

	//try to open the port for reading and writing
	check(sp_open(m_port, SP_MODE_READ_WRITE));
	//if there are no problems and we get to this point, set the port flag to OPEN
	m_port_status = PORT_OPEN;

	//load the default configuration into the port
	check(sp_set_config(m_port, m_default_config));

	//---------------------------------------------------------------------------------------------------
	//CLEAR ANYTHING THAT MAY HAVE BEEN IN THE INCOMING BUFFER
	//---------------------------------------------------------------------------------------------------
	sp_flush(m_port, static_cast<sp_buffer>(SP_BUF_INPUT));

	//---------------------------------------------------------------------------------------------------
	//TRY TO SEND THE "ping\n" PACKET
	//---------------------------------------------------------------------------------------------------
	
	//result will store the number of bytes written
	result = check(sp_blocking_write(m_port, send, size, timout));
	
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
		int n = sp_blocking_read(m_port, &buffer[pos], 1, 1000);

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
		printf("Success, expected: \"pong\", received: %n\n", buffer);
		this->handshakeresult = true;
		//add the port name to the global list so no other instances try to use it
		g_ports_in_use.insert(portname);
		m_portName = portname;
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
        if (m_port_status == PORT_OPEN)
        {
                sp_flush(m_port, SP_BUF_INPUT);
                sp_flush(m_port, SP_BUF_OUTPUT);
        }
}

void SerialComm::adjustPollingRate(float rate) const {
	/* \brief 	This function takes an int as a parameter and simply sends
	 * 		it to the connected arduino controller to control how many
	 * 		times per second a reading is generated.
	 */

	//first, check to make sure the port is open
	if(m_port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string adjust_command = "adjust," + std::to_string(rate) + "\n";

	//send the command
	(void) sp_blocking_write(m_port, adjust_command.c_str(), adjust_command.size(), 1000);

}

void SerialComm::removeSensor(const std::string& sensorName)
{
	/* \brief 	This function takes a string as a parameter and uses it to
	 * 		prompt the connected microcontroller to stope reading and
	 * 		sending data for it.
	 */

	//first check to make sure the port is open
	if (m_port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string remove_command = "remove," + sensorName + "\n";

	//send the command
	(void) sp_blocking_write(m_port, remove_command.c_str(), remove_command.size(), 1000);
}

void SerialComm::addSensor(const std::string& sensorName, int pin)
{
	/* \brief 	This function takes a name and a pin number as parameters
	 * 		in order to send a command to the microcontroller to
	 * 		start generating data for this sensor.
	 */

	std::cout << "oh boyyy we are trying to add with the arduinoo\n";
	//first make sure the port is open
	if (m_port_status == PORT_CLOSED)
	{
		throw std::runtime_error("Error: Attempt to send command over closed port");
	}

	//generate the command to send
	std::string add_command = "add," + sensorName + "," + std::to_string(pin) + "\n";

	//send the command
	(void) check(sp_blocking_write(m_port,add_command.c_str(), add_command.size(), 1000));

}

void SerialComm::readDataFrame(std::vector<std::unique_ptr<Sensor>>& sensors)
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
         *              This function is designed to run on a continuous
         *              background thread - updating each sensor reading in the
         *              project with new values as they are transmitted.
         */


        //quick check to make sure the port is opened for communication
	if (m_port_status == PORT_CLOSED)
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

    	// read as much data as possible in a single blocking call
    	// - buffer size is limited to prevent overflow

	char buffer[64];
	int pos = 0;

	while (pos < sizeof(buffer)-1) {
		int n = sp_blocking_read(m_port, &buffer[pos], 1, 1000);
		if (n <= 0) {return;}
		if (buffer[pos] == '\n') {break;}
		pos++;
	}

	flush();

	//then strip away any carriage return, if there is one, and null-terminate the array.
	buffer[strcspn(buffer, "\r\n")] = '\0';

	//--------------------
	//PARSE
	//--------------------

	const char* ptr = buffer;
	const char* start = buffer;
	std::size_t index = 0;

	while (true) {
		//if we are the the end of a value (comma) or the dataframe (null)
		if (*ptr == ',' || *ptr == '\0')
		{
			//then between the pointers start and ptr, there is a value we want to record
			int value = 0;
			std::from_chars(start,ptr,value);

			//safeguard the index of the sensor vector:
			if (index < sensors.size())
			{
				//write this reading to the appropriate sensor
				sensors[index]->setReading(value);
				//index only increments when this inner loop execute, since it represents one reading
				index++;
			}

			//if ptr was a null character, the dataframe is completely parsed, so we exit
			if (*ptr == '\0') {break;}

			//we've recorded a reading, and now we move start up to the first index
			start = ptr + 1;
		}
		//move ptr forward once in all cases.
		ptr++;
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
	if (m_port_status == PORT_CLOSED)
	{
		std::cout << "port is already closed, so the reset cannot be conducted\n";
		
		throw std::runtime_error("Error: Attempt to reset device through a closed port");
	}

	//generate the command to send
	std::string reset_command = "reset\n";

	//send the command
	(void) check(sp_blocking_write(m_port,reset_command.c_str(), reset_command.size(), 1000));
}

bool SerialComm::writeString(const std::string& str)
{
    if (!m_port) return false;       // port is your opened serial port
    std::string s = str + "\n";    // append newline if your ESP32 expects it
    int written = sp_blocking_write(m_port, s.c_str(), s.size(), 1000);
    return written == (int)s.size();
}

void SerialComm::cleanPort()
{
	//do nothing if there is no active port to clean
	if (m_port_status == PORT_CLOSED) return;

                sp_flush(m_port, static_cast<sp_buffer>(SP_BUF_INPUT | SP_BUF_OUTPUT));
                check(sp_close(m_port));
		sp_free_port(m_port);

		//free the name of the port from the global list
		g_ports_in_use.erase(m_portName);


		//reset member attributes, because this instance might yet be used for 
		//a new port with a different configuration.
		m_port = nullptr;
                m_port_status = PORT_CLOSED;
		m_portName.clear();
}

/* Helper function for error handling. */
int SerialComm::check(const enum sp_return result)
{
        std::string error_message = sp_last_error_message();
        switch (result) {
        case SP_ERR_ARG:
                throw std::runtime_error("Error: Invalid argument, fuck you.\n");
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


