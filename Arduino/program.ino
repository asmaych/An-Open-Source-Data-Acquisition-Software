//including map because we will use it to keep track of
//which sensor names go with which pin numbers
#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>

//this will be used to map sensor names to pin numbers
std::map<String, int> sensorMap;

//set some variables to keep track of elapsed time
unsigned long lastSampleTime = 0;

//default sample rate set to 50ms -> 20hz
unsigned long sampleRate = 50;

//vector to store an entire frame of data before sending
std::vector<String> frameOrder;

//defining a set size based on maximum expected command size
#define INPUT_SIZE 30

#define MAX_SENSORS 15

int readSensor(String name)
{
  /* \brief This function takes a String address as a parameter.
   *        it then uses the hashmap sensorMap to locate the pin
   *        associated with the name. The pin is read, and the 
   *        value is returned.
   */

  //if found, then we read from the pin
  if (sensorMap.find(name)!= sensorMap.end())
  {
    return(analogRead(sensorMap[name]));
  }

  //otherwise, we return -1 to indicate an error
  return -1;

}

void sendDataPacket()
{
  String tempFrame = "";
  //--------------------------------------------------------
  //Loop through the vector to ensure proper order
  //--------------------------------------------------------
  for (size_t i = 0; i < frameOrder.size(); i++)
  {
    //get the reading from readSensor()
    int reading = readSensor(frameOrder[i]);
    tempFrame += String(reading);

    //only add a comma separator when it's not the last value
    if (i < frameOrder.size() -1)
    {
      tempFrame += ",";
    }

  }



  //-------------------------------------------------------
  //Send the data frame
  //-------------------------------------------------------
  Serial.println(tempFrame);
}

void processCommand()
{

  //--------------------------------------------------------
  //Read the command into a char array
  //--------------------------------------------------------

  //define an input buffer to read the command
  //add 1 for null termination space
  char input[INPUT_SIZE+1];

  //read contents into input buffer.
  byte size = Serial.readBytesUntil("\n", input, INPUT_SIZE);

  //null-terminate the buffer that we wrote the command to:
  input[size] = 0;

  //--------------------------------------------------------
  //Parse the first argument of the command
  //--------------------------------------------------------

  //replace commas with null characters
  //command will point to the first character of the first
  //argument: 
  //"add"
  //"remove"
  //"adjust"
  //"ping" 
  char* command = strtok(input, ",");

  //if the first token is null for some reason, return
  if (!command) return;
  
  //---------------
  //CASE: "ping"
  //---------------
  if (strcmp(command, "ping") == 0)
  {
    Serial.println("pong");
  }
  //---------------
  //CASE: "add"
  //---------------
  else if (strcmp(command, "add") == 0)
  {
    //--------------------
    //Get second parameter
    //--------------------
    char* sensorname = strtok(NULL, ",");

    //--------------------
    //Get third parameter
    //--------------------
    char* pin = strtok(NULL,",");

    //check to see if we got actual values for each parameter
    //use short-circuiting AND
    if (sensorname && pin)
    {
      //push the sensor name to the vector that keeps track
      //of sensor reading order
      frameOrder.push_back(sensorname);

      //add the mapping to the map that associates sensors with
      //pin numbers
      sensorMap[sensorname] = atoi(pin);
    }
    else
    {
      //TODO implement feedback handling later
      //Serial.println("Error: misconfigured command \"add\"");
      return;
    }

  }
  //---------------
  //CASE: "remove"
  //---------------
  else if (strcmp(command, "remove") == 0)
  {
    //--------------------
    //Get second parameter
    //--------------------
    char* sensorname = strtok(NULL, ",");

    //check to see if the parameter is not null:
    if (sensorname)
    {
      //remove the sensor from the hashmap
      sensorMap.erase(sensorname);

      //remove it from the vector. using loops because
      //I don't know any other way
      for (size_t i = 0; i < frameOrder.size(); i++)
      {
        //if the two strings are equal
        if (strcmp(frameOrder[i],sensorname) == 0)
        {
          //we erase the entry from the vector at index i
          frameOrder.erase(frameOrder.begin() + i);
          break;
        }
      }

    }
    else
    {
      //TODO implement feedback handling later
      //Serial.println("Error: misconfigured command \"remove\"");
      return;
    }
  }
  //---------------
  //CASE: "adjust"
  //---------------
  else if (strcmp(command, "adjust") == 0)
  {
    //--------------------
    //Get second parameter
    //--------------------
    char* newsamplerate = strtok(NULL, ",");

    //check for not-nullness
    if (newsamplerate)
    {
      //set the new sampling rate
      sampleRate = atoi(newsamplerate);
    }
    else
    {
      //TODO implement feedback handling later
      //Serial.println("ERROR: misconfigured command \"adjust\"");
      return;
    }
  }
  else
  {
    //TODO implement feedback handling later
    //Serial.println("ERROR: no such command");
    return;
  }
  
}

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  //--------------------------------------------------------
  //Read and handle whatever is in the input stream:
  //--------------------------------------------------------

  if (Serial.available())
  {
    //call the function to handle the input
    processCommand();
  }

  //--------------------------------------------------------
  //After checking for interrupts, send a packet of data
  //--------------------------------------------------------

  //store the current elapsed time for comparison
  unsigned long timeNow = millis();
  
  //if the time elapsed between the last sample and now is 
  //sufficient to require another sample, we get another.
  if (timeNow - lastSampleTime >= sampleRate)
  {
    //make sure to check if we actually have any sensors to
    //send readings from.
    if (!sensorMap.empty())
    {
      sendDataPacket();
    }
    //now update the lastSampleTime to reflect the new sample
    lastSampleTime = timeNow;
  }
}
>>>>>>> Joachim
