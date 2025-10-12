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
          const std::pair<double, double>& inputVoltageRange = {0.0, 5.0},
          const std::pair<double, double>& outputVoltageRange = {0.0, 10.0},
          const std::string& datasheet = "N/A",
          SerialComm* serial);

    //Setting the serial communication link with SerialComm
    void attachSerial(SerialComm* comm);

    //Reading sensor data from the serial port
    bool readFromSerial();

   //Getters
  int getID() const{
      return id;
    }
  std::string getType() const{
        return type;
    }
  std::string getName() const{
        return name;
    }
  std::string getUnit() const{
        return unit;
    }
  std::pair<double, double> getInputRange() const{
        return inputVoltageRange;
    }
  std::pair<double, double> getOutputRange() const{
        return outputVoltageRange;
    }
  std::string getDatasheet() const{
        return datasheet;
    }
  double getLastReading() const{
        return lastReading;
    }

  //Setters
  void setName(const std::string& newName){
      name = newName;
    }
  void setType(const std::string& newType){
      type = newType;
    }
  void setUnit(const std::string& newUnit){
      unit = newUnit;
    }
  void setInputRange(double min, double max){
      inputMin = min;
      inputMax = max;
    }
  void setOutputRange(double min, double max){
      outputMin = min;
      outputMax = max;
    }
  void setDatasheet(const std::string& path){
      datasheet = path;
    }

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
  double lastReading;  //Stores latest sensor reading
  SerialComm* serial; //Pointer to SerialComm for reading data
};
