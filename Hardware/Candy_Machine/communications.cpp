#include <Arduino.h>
#include "hardware_operations.h"
// -------------------------------------------------------------------------------------------- //
//Serial Paramaters
#define SERIAL_BAUD_RATE 9600
// ID10T Commands Types
#define TRANS_TYPE_COMMAND 0x7e // The command trans type is denoted by a Tilde '~'
#define TRANS_TYPE_ACKNOWLEDGE 0x40 //The acknowledge trans type is denoted by an "AT" symbol '@'
#define TRANS_TYPE_EVENT 0x25 //The event trans type is denoted by a percent '%'
// ID10T Commands
#define ESTABLISH_CONNECTION 0X45 // This command is denoted by a capital 'E' 
#define DISPENSING_CANDY 0x49 // This command is denoted by a capital 'I' 
#define RESET 0x51 // This command is denoted by a capital 'Q' 
// ID10T Acknowledgements
#define CONNECTION_ESTABLISHED [0x40,0X65,0x73] // This ACK is denoted by a lowercase 'e'
#define DISPENSING_CANDY [0x40,0X69,0x79] // This ACK is denoted by a lowercase 'i'
#define RESETTING [0x40,0X71,0x44] // This ACK is denoted by a lowercase 'q'
// ID10T Buffer Constants
#define SERIAL_INCOMING_BUFFER_SIZE 64
// ID10T Buffer Integers
char serialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int serialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int serialIncomingWritePointer = 0; // Index where to write next byte

int ResetNow = 0; 
// -------------------------------------------------------------------------------------------- //
bool ResetToggle () {   // When true, the machine will reset itself
  if (ResetNow = RESET) {
    return true
  } else {
    return false
  } 
}
// -------------------------------------------------------------------------------------------- //
void SetUpCommunications () {   // set up the serial library at baud defined, wait for serial to initilize 
  // Set up Serial
  Serial.begin(SERIAL_BAUD_RATE);
  // Wait for Serial           
  while (!Serial) {
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
int ListenOnSerial () {   // Listen on Serial fallback for Buffer
  int incomingByte = 0;
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    //Serial.println(incomingByte, HEX); // For debug only
  }
  return incomingByte;
}
// -------------------------------------------------------------------------------------------- //
void WriteOnSerial (int SendOnSerial) {   // Output to the Serial BUS
  Serial.write(SendOnSerial);
}
// -------------------------------------------------------------------------------------------- //
bool IsConnectionEstablished() {    // Determine if there is a program to talk to
  if (ListenOnSerial() == ESTABLISH_CONNECTION) {
    return true;
    } else {
      return false;
  }
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () { // Wait for a program to talk to
  while (IsConnectionEstablished() == false) {
    IsConnectionEstablished();
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
void readSerial() { // Generat a circular buffer to store incoming comamnds for later interpretation
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
char pullByteOffQueue() {   //read the bytes on the buffer
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
void processIncomingQueue() {   //interpret the byte pulled from the cue and execute the command
  // Pull off a single command from the queue if command has enough bytes.
  // For simplicity sake, all commands will be a total of 3 bytes (indicating command type, command id, command parameter)
  if (serialIncomingQueueFillAmt > 2) { // Having anything more than 2 means we have enough to pull a 3 byte command.
    // Check if first byte in queue indicates a command type (if not, throw it out and don't process more until next time processIncomingQueue is called)
    char byteRead = pullByteOffQueue();
    if (byteRead == TRANS_TYPE_COMMAND) {
      // Check if next byte matches command indicator (if it does ignore byte and pull next)
      byteRead = pullByteOffQueue();
      if (byteRead == TRANS_TYPE_COMMAND) {
        byteRead = pullByteOffQueue();
      } 
      } else if (byteRead == TRANS_TYPE_ACKNOWLEDGE){
        // Check if next byte matches command indicator (if it does ignore byte and pull next)
        byteRead = pullByteOffQueue();
        if (byteRead == TRANS_TYPE_ACKNOWLEDGE) {
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
            WriteOnSerial(DISPENSING_CANDY);
            ControlMotor (byteRead);
          break;
          case RESET:

            byteRead = pullByteOffQueue();
            WriteOnSerial (RESETTING)
            int ResetNow = Reset;
          break;
          case 
        
        }
      }
    }
  }
}
// -------------------------------------------------------------------------------------------- //
