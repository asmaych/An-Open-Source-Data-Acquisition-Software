#pragma once

#include <string>
#include <atomic> 
#include <mutex>

/* #include <atomic>: for thread-safe reading, it ensures that 
 * operations on a variable are performed atomically 
 * (uninterruptible and safe for multithreaded environments).
 *
 * #include <mutex>: provides us with a sychronization primitive 
 * that is used to protect the shared data from being accessed 
 * by multiple threads simultaneously.
 */

class Sensor 
{

public: 

	//Constructor: initializes a sensor with a name and a pin number
	Sensor(const std::string& name, int pin);
	
	//---------------------------------------------------------------
	//GETTERS
	//---------------------------------------------------------------
	
	std::string getName() const;
	int getPin() const;
	 
	/* a getter to  get the current reading (thread-safe meaning 
	 * it can be called from multiple threads (a sequence of 
	 * instructions that can run concurrently within a program)  
	 * at the same time without causing errors, crashes, or incorrect 
	 * results.
	 */
	int getReading() const;
	
	//---------------------------------------------------------------
	//SETTERS
	//---------------------------------------------------------------

	// a setter for current reading (again thread-safe)
	void setReading(int value);

	void setSelected(bool sel) { m_selected = sel; }
        bool isSelected() const { return m_selected; }


private:
	
	std::string m_name;
	int m_pin;
	std::atomic<int> m_reading; //thread-safe reading value
	bool m_selected = true;
};
