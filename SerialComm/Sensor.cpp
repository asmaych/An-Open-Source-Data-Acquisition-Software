#include "Sensor.h"
#include <iostream>

Sensor::Sensor()
      {
        id(0);
        name("Unnamed");
        type("Unkown"); 
        unit(""); 
        inputVoltageRange({0.0, 5.0}); 
        outputVoltageRange({0.0, 10.0}); 
        datashaaet("N/A"); 
        serial=nullptr; 
        displayStatus=nullptr;
      }

Sensor::Sensor(int sensorID, std::string& sensorName, std::string& sensorType, std::string sensorUnit, std::pair& sensorInputRange, std::pair& sensorOutputRange, 
                std::string& path, SerialComm* serialPtr, DisplayStatus* dispStatus)
      {
          id = sensorID;
          name = sensorName;
          type = sensorType;
          unit = sensorUnit;
          inputVoltageRange = sensorInputRange;
          outputVoltageRange = sensorOutputRange;
          datasheet = path;
          serial = serialPtr;
          displayStatus = dispStatus;

          if(displayStatus){
            displayStatus->AddMessage(id, "Sensor", "Sensor '" + name + "' created successfully.");
      }


//Setters
  void Sensor::setName(const std::string& newName){
      name = newName;
      if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor name set as " + name + "'.");
      }
    }
  void Sensor::setType(const std::string& newType){
      type = newType;
      if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor type set as '" + type + "' for " + name + " sensor.");
      }
    }
  void Sensor::setUnit(const std::string& newUnit){
      unit = newUnit;
      if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor unit set as '" + unit + "' for " + name + " sensor.");
      }
    }
  void Sensor::setInputRange(double min, double max){
      inputVoltageRange = {min,max};
      if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor input voltage range set as '" + inputVoltageRange + "' for " + name + " sensor.");
      }
    }
  void Sensor::setOutputRange(double min, double max){
     outputVoltageRange = {min,max};
     if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor output voltage range set as '" + outputVoltageRange + "' for " + name + " sensor.");
      }
    }
  void Sensor::setDatasheet(const std::string& newPath){
      datasheet = newPath;
      if(displayStatus){
          displayStatus->AddMessage(id, "Sensor", "Sensor datasheet set as '" + datasheet + "' for " + name + " sensor.");
      }
    }


  //Getters
  int Sensor::getID() const{
        return id;
      }
  std::string Sensor::getType() const{
        return type;
    }
  std::string Sensor::getName() const{
        return name;
    }
  std::string Sensor::getUnit() const{
        return unit;
    }
  std::pair<double, double> Sensor::getInputRange() const{
        return inputVoltageRange;
    }
  std::pair<double, double> Sensor::getOutputRange() const{
        return outputVoltageRange;
    }
  std::string Sensor::getDatasheet() const{
        return datasheet;
    }


  //Reading
  std::string Sensor::readSensorData(){
    if(serial==nullptr){
        if(displayStatus){
          displayStatus->AddError(id, "Sensor", "SerialComm not connected for " + name + ".");
        }
    return "";
    }


//ASMAAAAAAA!! YOU ARE STILL MISSING THIS PART


  //Display all sensor info
  void Sensor::DisplayInfo(){
    std::cout << "\n-----------------------------------------\n";
//what about using display here?? just by removing the owner would that be better?!!
    std::cout << "Sensor ID: " << id << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Type: " << type << std::endl;
    std::cout << "Input Voltage Range in V: " << inputVoltageRange << std::endl;
    std::cout << "Output Voltage Range in V: " << outputVoltageRange << std::endl;
    std::cout << "Datasheet: " << datasheet << std::endl;
    std::cout << "-----------------------------------------\n";

    if(displayStatus){
      displayStatus->AddMessage(id, "Sensor", "Sensor info displayed for " + name + ".");
    }
  }



