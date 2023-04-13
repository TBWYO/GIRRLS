#include <Arduino.h>
#include "hardware_operations.h"

#define SERIAL_BAUD_RATE 9600

void SetUpCommunications () {
  // Set up Serial
  Serial.begin(SERIAL_BAUD_RATE);           // set up the serial library at baud defined
  while (!Serial) {
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
int ListenOnSerial () {
  int incomingByte = 0;
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    //Serial.println(incomingByte, HEX); // For debug only
  }
  return incomingByte;
}
// -------------------------------------------------------------------------------------------- //
void WriteOnSerial (int SendOnSerial) {
  Serial.write(SendOnSerial);
}
// -------------------------------------------------------------------------------------------- //

