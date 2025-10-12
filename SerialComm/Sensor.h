#pragma once
#include <iostream>
#include <string>
#include <vector>

class Sensor
{
  Public:
    Sensor(int id, 
          const std::string& type,
          const std::string& name,
          const std::string& unit,
          const std::pair<double, double>& inputVoltageRange,
          const std::pair<double, double>& outputVoltageRange,
          const std::string& datasheet = "N/A",
          SerialComm* serial,
          DisplayStatus* displayStatus);

    //Setters
    void setName(const std::string& newName){}
    void setType(const std::string& newType){}
    void setUnit(const std::string& newUnit){}
    void setInputRange(double min, double max){}
    void setOutputRange(double min, double max){}
    void setDatasheet(const std::string& path){}
  
     //Getters
    int getID() const{}
    std::string getType() const{}
    std::string getName() const{}
    std::string getUnit() const{}
    std::pair<double, double> getInputRange() const{}
    std::pair<double, double> getOutputRange() const{}
    std::string getDatasheet() const{}



    //Reading
    std::string readSensorData();
    //Display all sensor info
    void DisplayInfo() const;          


 Private:
  int id; // Unique sensor ID
  std::string name; //"TMP45"
  std::string type; // "Tempreture"..
  std::string unit; // "°C..."
  std::pair<double, double>& inputVoltageRange; //{0.0 , 5.0}
  std::pair<double, double>& outputVoltageRange; //{0.0 , 12.0}
  std::string datasheet; //Link or file path to datasheet
  //double lastReading;  //Stores latest sensor reading
  SerialComm* serial; //Pointer to SerialComm for reading data
  DisplayStatus* displayStatus;
};
