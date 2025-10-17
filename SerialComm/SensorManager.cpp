#pragma once
#include <unordered_map>
//a hashmap for fast ID lookup
#include <string>
#include <iostream>
#include "Sensor.h"
#include "DisplayStatus.h"

class SensorManager {
public:
   // I call the constructor that accepts a pointer to a DisplayStatus object (nullptr by default)
    SensorManager(DisplayStatus* displayStatus = nullptr)
        : display(displayStatus) {}

    // Add a new sensor to the manager
    void addSensor(Sensor* sensor) {
        int id = sensor->getID();
        // I store the sensor in a hashmap, keyed by ID
        sensors[id] = sensor;
        // display a message of sensor XX added successfully 
        if(display) {
            display->AddMessage(id, "SensorManager", "Sensor '" + sensor->getName() + "' added successfully.");
        }
    }

    // Get sensor by ID (returns nullptr if not found)
    Sensor* getSensor(int id) const {
        // "find" searches the hashmap for a sensor with the provided ID
        // auto it = sensor.find(id): Automatically deduces the type of the variable it based on the return type of sensors.find(id). 
        // In this case, it will be an iterator pointing to the found element (a sensor pointer) or sensors.end() if the element is not found (nullptr).
        auto it = sensors.find(id);
        if(it != sensors.end())
            return it->second;
        return nullptr;
    }

    // Remove sensor by ID
    void removeSensor(int id) {
        // erase (ID) => removes the sensor with this ID? and returns number of elements removed (0, 1).
        if(sensors.erase(id) > 0 && display) {
            display->AddMessage(id, "SensorManager", "Sensor removed.");
        }
    }

    // Display all sensors info
    void displayAllSensors() const {
        // I will call displayInfo() on each sensor to print its info
        for(const auto& pair : sensors) {
            pair.second->DisplayInfo();
        }
        if(display)
            display->AddMessage(-1, "SensorManager", "Displayed all sensors info.");
    }

private:
    std::unordered_map<int, Sensor*> sensors; // stores all sensors, keyed by ID
    DisplayStatus* display; // optional pointer to displayStatus to log actions
};
