#define BUTTON_PIN 2  // Button input pin

bool beginReceived = false;  // Tracks if "Begin" was received

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (!beginReceived) {
      // Echo back everything until "Begin"
      Serial.println(input);
      if (input.equalsIgnoreCase("Begin")) {
        beginReceived = true;
        flushSerial();
      }
    } else {
      // After "Begin", only respond to "Request"
      if (input.equalsIgnoreCase("Request")) {
          int buttonState = digitalRead(BUTTON_PIN);
          Serial.write(buttonState == LOW ? 1 : 0);
      }
    }
  }
}

void flushSerial() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

