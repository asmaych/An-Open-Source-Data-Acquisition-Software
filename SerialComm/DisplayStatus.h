#pragma once
//pragma once is used to make sure that the header file is included only once per compilation (if displaystatus was already included skip it)
#include <iostream>
#include <vector>
#include <string>

/* 
 * This class is a "status manager". It keeps track of all messages, warnings, and errors that different parts of DAQ send out (right now only - SerialComm).
 * Each message is connected to an "owner" like SerialComm, and it also has an ID, in case we want to identify or update specific messages later, a type (info, warning...) and a content.
*/

class DisplayStatus
{
  public:
     struct Message
     {
      int id; //the message id
      std::string owner; //the class that sent the message
      std::string content; //the message text
      std::string type; //the message type - warning, info, error...
     };
  
    DisplayStatus(); //default
    
    //Add a general INFO message to the vector with the parameters (id, owner and msg) using push_back = append = add to the end
    void AddMessage(int id, const std::string& owner, const std::string& message)
    {
      messages.push_back({id, owner, message, "INFO"});
      std::cout << "[INFO][" << owner << "][" << id << "]: " << message << std::endl;
    }
    
    //Add a warning message
    void AddWarning(int id, const  std::string& owner, const  std::string& message)
    {
      messages.push_back({id, owner, message, "WARNING"});
      std::cout << "[WARNING][" << owner << "][" << id << "]: " << message << std::endl;
    }
    
    //Add an error message 
    void AddError(int id, const  std::string& owner, const  std::string& message)
    {
      messages.push_back({id, owner, message, "ERROR"});
      std::cout << "[ERROR][" << owner << "][" << id << "]: " << message << std::endl;
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

  //clear all messages
    void clear()
    {
      messages.clear();
      std::cout << "[INFO] DisplayStatus: All messages cleared." << std::endl;
    }
  
  private:
    // we store all message entries in a dynamic array (vector) that can grow in size as needed, 
    // where we can look up a message by owner or ID with a time complexity of O(n) cause it has to scan the vector
    std::vector<Message> messages; 

    /* On the other hand, a hashmap stores elements as key-value pairs, lookup by key is O(1) average (fast)
       It can directly access messages by ID or owner (key) without scanning
       Its memory is a bit higher than a vector because it stores  uckets, pointers, and metadata for the hash table.
       => For me, since our DAS will have hundreds or low thousands of messages, then a vector is fine and simple, but if we re expecting tens or hundreds of thousands
       of messages and frequent queries by ID and owner, then we need to go with the hash Map! (Asma)
    WHAT DO YOU THINK JOEEEEE!!!!
    */

    /* The size of one message:
        int id - 4 bytes
        str owner - 32 bytes
        str content - 32 bytes
        str type - 32 bytes
     => Total = 100 bytes ( can vary depending on the str length)
      Let's assume the vector capacity is 1.5 x current size = 50%  in worst case while growing.
      So, if we have 1000 messages => 1000 x 100 bytes = 100 000 bytes (approximately 100KB) 
      with extra vector capaicty (approx 100KB)
      Overhead will be approx 50%

      In the case of the Hashmap ( data: ~100KB, extra pointers & buckets: ~20-40KB, Total: ~120-140KB)
      */
};
    

