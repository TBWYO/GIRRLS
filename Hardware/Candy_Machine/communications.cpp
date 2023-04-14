#include <Arduino.h>

#include "hardware_operations.h"
// -------------------------------------------------------------------------------------------- //
/* The following constants are derived from the ID10T protocal
 * The protocal uses 3 bytes to communicate {(ID)(CMD)(PARAM)} -- ID, Command, Parameter
 * Every command recieves needs an acknowledgement sent to the program.
 * Acknowledgements are structued thusly; {(ID)
*/
#define SERIAL_BAUD_RATE 9600
#define ESTABLISH_CONNECTION 0x45 // Value sent from python program to establish communications (capitol "E")
#define DISPENSE_ACTIVATE_COMMAND 0x49 // Capital 'I'
// #define MOTOR_ROTATE_TRUE 0X44 // Capital "D" If 'D' rotate, else nothing (not used here, present for completeness)
#define COMMAND_ACKNOWLEDGED 0x54 //Capital 'T'
#define SERIAL_INCOMING_BUFFER_SIZE 64
#define COMMAND_INDICATOR_BYTE 0x7E // Tilde '~'
// -------------------------------------------------------------------------------------------- //
char serialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int serialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int serialIncomingWritePointer = 0; // Index where to write next byte
// -------------------------------------------------------------------------------------------- //
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
bool IsConnectionEstablished() {
  if (ListenOnSerial() == ESTABLISH_CONNECTION) {
    return true;
    } else {
      return false;
  }
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () {
  while (IsConnectionEstablished() == false) {
    IsConnectionEstablished();
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
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
// -------------------------------------------------------------------------------------------- //
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
// -------------------------------------------------------------------------------------------- //
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
          case DISPENSE_ACTIVATE_COMMAND: // Controls the motor movement
            // Pull 3rd byte for command that can be use as specific parameters
            byteRead = pullByteOffQueue();
            
            // Call command specific function while passing parameter byte
            WriteOnSerial(COMMAND_ACKNOWLEDGED);
            ControlMotor (byteRead);
          break;
        
        }
      }
    }
  }
}
