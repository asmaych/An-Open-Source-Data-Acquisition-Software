#pragma once
#include <iostream>
#include <vector>
#include <string>

/* 
 * This class is basically my "status manager". It keeps track of all messages, warnings, and errors that different parts of DAQ send out (right now we just have - SerialComm).
 * Each message is connected to an "owner" like SerialComm, and it also has an ID, in case we want to identify or update specific messages later.
*/
class DisplayStatus
{
  public:
     struct Message
     {
      int id;
      std::string owner; //the class that sent the message
      std::string content; //the message text
      std::string type; //the message type - warning, info, error...
     };
  
    DisplayStatus();
    
    //Add a general message with id, owner and msg
    void AddMessage(int id, const std::string& owner, const std::string& message)
    {
      message.push_back({id, owner, message, "INFO"});
      std::cout << "[INFO][" << owner << "][" << id << "]: " << message << std:endl;
    }
    
    //Add a warning message
    void AddWarning(int id, const wxString& owner, const wxString& message);
    {
      message.push_back({id, owner, message, "WARNING"});
      std::cout << "[WARNING][" << owner << "][" << id << "]: " << message << std:endl;
    }
    
    //Add an error message 
    void AddError(int id, const wxString& owner, const wxString& message);
    {
      message.push_back({id, owner, message, "ERROR"});
      std::cout << "[ERROR][" << owner << "][" << id << "]: " << message << std:endl;
    }
    
    //showAll will print all the messages stored so far with all the details (owner, content...)
    void showAll() const
    {
      std::cout << "DisplayStatus: All messages " << std::endl;
      for (const auto& message: messages)
      {
        std::cout << "[" << message.type << "][" << message.owner << "][" << message.id << "]: " << message.content << std::endl;
      }
      std::cout << "------------------------------------------------------------" <<std::endl;
    }
    
    // Get all messages that belong to a specific owner (eg SerialComm)
    std::vector<Message> GetMessagesByOwner(const std::string& owner) const
    {
      std::vector<Message> filtered;
      for(const auto& message: messages)
      {
          if(message.owner == owner)
              filtered.push_back(message);
      }
    return filtered;
    }
    
    void clear()
    {
      message.clear();
      std::cout << "[INFO] DisplayStatus: All messages cleared." << std::endl;
    }
  
  private:
      std::vector<Message> messages; //we store all message entries in a vector
};
    

