#include <Arduino.h>

void setup() {
  Serial.begin(9600);

  // wait for serial connection
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.println("ESP32 ready!");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input == "Request") {
            int sensorValue = analogRead(2);  // read potentiometer
            Serial.println(sensorValue);       // send numeric value
        } 
        else {
            Serial.print("Echo: ");
            Serial.println(input); // echo any other input
        }
    }
}
 
