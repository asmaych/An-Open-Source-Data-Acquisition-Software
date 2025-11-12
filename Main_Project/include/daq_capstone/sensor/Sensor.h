#ifndef SENSOR_H
#define SENSOR_H
#include <string>
#include <atomic> 
#include <mutex>

/* #include <atomic>: for thread-safe reading, it ensures that operations on a variable are performed atomically
   (uninterruptible and safe for multithreaded environments).

   #include <mutex>: provides us with a sychronization primitive that is used to protect the shared data from being accessed 
   by multiple threads simultaneously.
*/

class Sensor 
{

// Here i am building a simple representation of a sensor with a name and an ID.

public: 

	//Constructor: initializes a sensor with a name and an ID
	Sensor(const std::string& name, int id);

	//Getters
	std::string getName() const;
	int getID() const;
	 
	/* a getter to  get the current reading (thread-safe meaning it can be called from multiple threads (a sequence of instructions
	   that can run concurrently within a program)  at the same time without causing errors, crashes, or incorrect results.
	*/
	int getReading() const;
	
	// a setter for current reading (again thread-safe)
	void setReading(int value);

private:
	
	std::string name;
	int id;
	std::atomic<int> reading; //thread=safe current reading
};

#endif
