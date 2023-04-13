#define PIN_LED   13
#define BLINK_RATE 1000
#define SERIAL_INCOMING_BUFFER_SIZE 64
#define COMMAND_INDICATOR_BYTE '~'
// ================================================================================
unsigned long timeNow = 0;
unsigned long timeStateChange = millis();
bool ledBlinkIsOn = false;
bool ledIsLit = false;
char serialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int serialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int serialIncomingWritePointer = 0; // Index where to write next byte
// ================================================================================
void setup() {
  // Configure Pins
  pinMode(PIN_LED, OUTPUT);

  // Configure Serial
  Serial.begin(9600);
  while (!Serial); // Wait until Serial is initialized

  
}
// ================================================================================
void loop() {
  blinkLed();
  readSerial();
  processIncomingQueue();
  runMotor();
  readSensors();
}
// ================================================================================
void blinkLed() {
  timeNow = millis();

  if (timeNow > timeStateChange && ledBlinkIsOn == true) {
    if (ledIsLit == true) {
      digitalWrite(PIN_LED, LOW);
      ledIsLit = false;
    } else {
      digitalWrite(PIN_LED, HIGH);
      ledIsLit = true;
      //Serial.println("On");
    }
    timeStateChange = millis() + BLINK_RATE;
  } else if (ledBlinkIsOn == false) {
    // Make sure is off
    digitalWrite(PIN_LED, LOW);
    ledIsLit = false;
  }
}
// ================================================================================
void readSerial() {
  int bytesToRead = Serial.available();
  if (bytesToRead > 0) {
    // Dump each byte to queue
    for (int i = 0; i < bytesToRead; i++) {
      // Check to be sure room still exists in buffer
      if (serialIncomingQueueFillAmt < SERIAL_INCOMING_BUFFER_SIZE) {
        serialIncomingQueue[serialIncomingWritePointer] = Serial.read();
        // Increase count of what is in buffer
        serialIncomingQueueFillAmt++;

        // Move where to write next byte (may need to loop back to start of array if end is full).
        // Note that end is based on a start index of 0 so 64 size would have index range is 0-63
        if (serialIncomingWritePointer == SERIAL_INCOMING_BUFFER_SIZE - 1) {
          // Loop back to first index
          serialIncomingWritePointer = 0;
        } else {
          serialIncomingWritePointer++;
        }
      }
    }
  }
}
// ================================================================================
void processIncomingQueue() {
  // Pull off a single command from the queue if command has enough bytes.
  // For simplicity sake, all commands will be a total of 3 bytes (indicating command type, command id, command parameter)
  if (serialIncomingQueueFillAmt > 2) { // Having anything more than 2 means we have enough to pull a 3 byte command.
    // Check if first byte in queue indicates a command type (if not, throw it out and don't process more until next time processIncomingQueue is called)
    char byteRead = pullByteOffQueue();
    if (byteRead == COMMAND_INDICATOR_BYTE) {
      // Check if next byte matches command indicator (if it does ignore byte and pull next)
      byteRead = pullByteOffQueue();
      if (byteRead == COMMAND_INDICATOR_BYTE) {
        byteRead = pullByteOffQueue();
      }

      // Make sure there is still a possible parameter byte
      if (serialIncomingQueueFillAmt > 0) {
        // Then see if byte matches command id; if not double bytes and do nothing until next time processIncomingQueue is called
        switch (byteRead) {
          case 'L': // Control LED (~L1 = Turn on blinking LED, ~L0 = Turn off blinking LED)
            // Pull 3rd byte for command that can be use as specific parameters
            byteRead = pullByteOffQueue();
            
            // Call command specific function while passing parameter byte
            controlLedMode(byteRead);
          break;
        }
      }
    }
  }
}
// ================================================================================
char pullByteOffQueue() {
  // Note that this function is not verifying bytes exist before pulling so you MUST be sure there is a usable byte BEFORE calling this function
  char returnValue = serialIncomingQueue[serialIncomingReadPointer];

  // consume up the byte read by moving the read pointer and decreasing fill amount
  if (serialIncomingReadPointer == SERIAL_INCOMING_BUFFER_SIZE - 1) {
    // Loop back to first index
    serialIncomingReadPointer = 0;
  } else {
    serialIncomingReadPointer++;
  }
  serialIncomingQueueFillAmt--;

  return returnValue;
}
// ================================================================================
void controlLedMode (char parameter) {
  // Set flag to perform specific action
  if (parameter == '1') { // Note that this is ascii 1 and not number 1
    // Turn LED on
    ledBlinkIsOn = true;
  } else if (parameter == '0') {
    // Turn LED off
    ledBlinkIsOn = false;
  }

  // Let other side know that you received and understood command
  if (parameter == '0' || parameter == '1') {
    Serial.write('!');
  }
}
// ================================================================================
void runMotor() {

}
// ================================================================================
void readSensors() {

}
