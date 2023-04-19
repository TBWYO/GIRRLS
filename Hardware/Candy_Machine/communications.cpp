#include <Arduino.h>
#include "hardware_operations.h"
#include "communications.h"
// -------------------------------------------------------------------------------------------- //
// Serial Paramaters
#define SERIAL_BAUD_RATE 9600
// ID10T Commands Types
#define TRANS_TYPE_COMMAND 0x7E // The command trans type is denoted by a Tilde '~'
#define TRANS_TYPE_ACKNOWLEDGE 0x40 //The acknowledge trans type is denoted by an "AT" symbol '@'
// ID10T Host Commands
int ESTABLISH_CONNECTION[3] = {0x7e,0x45,0x53}; 
int DISPENSE_CANDY[3] = {0x7e,0x49,0x44}; // This command is denoted by a capital 'I' 
#define RESET 0x51 // This command is denoted by a capital 'Q'
// ID10T Host Acknowledgements
char ESTABLISH_CONNECTION_SERIAL_RESPONSE[3] = {0x40,0x65,0x73};
char MOTOR_ROTATE_RESPONSE[3] = {0x40,0x69,0x79};
// ID10T Buffer Constants
#define SERIAL_INCOMING_BUFFER_SIZE 64
// ID10T Buffer Integers
char serialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int serialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int serialIncomingWritePointer = 0; // Index where to write next byte
int ResetNow = 0;
int CommsEstablishedSerial =0;
bool TransTypeCommand = false; 
bool TransTypeAcknowledge = false;
int PausedForAssistance = 0; 
// -------------------------------------------------------------------------------------------- //
bool ResetToggle () {   // When true, the machine will reset itself
  if (ResetNow == RESET) {
    return true;
  } else {
    return false;
  } 
}
// -------------------------------------------------------------------------------------------- //
bool IsConnectionEstablished = false;
// -------------------------------------------------------------------------------------------- //
bool IsProgramPaused () {
  if (PausedForAssistance = 1) {
    return true;
  } else {
    return false;
  }
}
// -------------------------------------------------------------------------------------------- //
void SetUpCommunications() {
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
void WriteOnSerial (char* SendOnSerialArray, int length) {   // Output to the Serial BUS
  Serial.write(SendOnSerialArray, length);
}
// -------------------------------------------------------------------------------------------- //

void readSerial () { // Generat a circular buffer to store incoming comamnds for later interpretation
  int bytesToRead = Serial.available();
  if (bytesToRead > 0) {
    // Dump each byte to queue
    for (int i = 0; i < bytesToRead; i++) {
      // Check to be sure room still exists in buffer
      if (serialIncomingQueueFillAmt < SERIAL_INCOMING_BUFFER_SIZE) {
        //int SerialReadByte = Serial.read();
        //if (SerialReadByte ==XON)
        serialIncomingQueue[serialIncomingWritePointer] = Serial.read();
        // Serial.println(serialIncomingQueue[serialIncomingWritePointer], HEX); // For debug only
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
char pullByteOffQueue () {   //read the bytes on the buffer
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
void processIncomingQueue () {   //interpret the byte pulled from the cue and execute the command
  // Pull off a single command from the queue if command has enough bytes.
  // For simplicity sake, all commands will be a total of 3 bytes (indicating command type, command id, command parameter)
  if (serialIncomingQueueFillAmt > 2) { // Having anything more than 2 means we have enough to pull a 3 byte command.
    // Check if first byte in queue indicates a command type (if not, throw it out and don't process more until next time processIncomingQueue is called)
    char byteRead = pullByteOffQueue();
     if (byteRead == TRANS_TYPE_COMMAND) {
      TransTypeCommand = true;
      TransTypeAcknowledge = false;
    } else if (byteRead == TRANS_TYPE_ACKNOWLEDGE) {
      TransTypeAcknowledge = true;
      TransTypeCommand = false;
    } else {
      byteRead = 0;
      TransTypeCommand = false;
      TransTypeAcknowledge = false;
    }
    if (TransTypeCommand) { 
      byteRead = pullByteOffQueue();
      if (byteRead == ESTABLISH_CONNECTION[1]) {
        byteRead = pullByteOffQueue();
        if (byteRead == ESTABLISH_CONNECTION[2]) {
          WriteOnSerial(ESTABLISH_CONNECTION_SERIAL_RESPONSE, 3);
          IsConnectionEstablished = true;
        }
      } else if (byteRead == DISPENSE_CANDY[1]) {
        byteRead = pullByteOffQueue();
          if (byteRead == DISPENSE_CANDY[2]) {
          ControlMotor(byteRead);
          WriteOnSerial(MOTOR_ROTATE_RESPONSE, 3);
          }
      } /*else if (byteRead == RESET) {
        byteRead = pullByteOffQueue();
        //WriteOnSerial(TRANS_TYPE_COMMAND);
        //WriteOnSerial(RESETTING);
        Restart();
      } */

    

       //Make sure there is still a possible parameter byte
      if (serialIncomingQueueFillAmt > 0) {
        // Then see if byte matches command id; if not double bytes and do nothing until next time processIncomingQueue is called
          
      }
}
}
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () { // Wait for a program to talk to
  while (!IsConnectionEstablished) {
    readSerial(); //listen on serial for the establish comms command from program
    processIncomingQueue(); //determine what was send over Serial
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
