#include <Arduino.h>
#include "hardware_operations.h"
// -------------------------------------------------------------------------------------------- //
// Serial Paramaters
#define SERIAL_BAUD_RATE 9600
// ID10T Commands Types
#define TRANS_TYPE_COMMAND 0x7E // The command trans type is denoted by a Tilde '~'
#define TRANS_TYPE_ACKNOWLEDGE 0x40 //The acknowledge trans type is denoted by an "AT" symbol '@'
#define TRANS_TYPE_EVENT 0x25 //The event trans type is denoted by a percent '%'
//ID10T Host Parameters
#define ESTABLISH_CONNECTION_SERIAL 0x42 // This parameter determines that communications will occur over Serial.
// ID10T Host Commands
#define ESTABLISH_CONNECTION 0X45 // This command is denoted by a capital 'E' 
#define DISPENSE_CANDY 0x49 // This command is denoted by a capital 'I' 
#define RESET 0x51 // This command is denoted by a capital 'Q'
// ID10T Client Events
#define JAM_OR_EPMTY 0x4a // Tell the program that candy was not dispensed
#define DISEPENSE_DETECTED 0x4d // Tell the program the candy WAS dispensed
#define CANDY_TAKEN 0x54 // Tell the program that one or more pieces of candy were taken
// ID10T Host Acknowledgements
#define CONNECTION_ESTABLISHED 0X65 // This ACK is denoted by a lowercase 'e'
#define DISPENSING_CANDY 0X69 // This ACK is denoted by a lowercase 'i'
#define RESETTING 0X71 // This ACK is denoted by a lowercase 'q'
// ID10T Client Acknowledgements
#define PAUSED_FOR_ASSISTANCE 0x6a // Pause the program until issue is resolved
#define DISPENS_NOTED 0x6m // Acknowledge that the program has counted the dispense
#define CANDY_TAKEN_NOTED 0x74 // Acknowledge that candy was taken, reduce count
// ID10T Buffer Constants
#define SERIAL_INCOMING_BUFFER_SIZE 64
// ID10T Buffer Integers
char serialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int serialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int serialIncomingWritePointer = 0; // Index where to write next byte
int ResetNow = 0;
int CommsEstablishedSerial = 0;
int CommandEnable = 0;
int AcknowledgeEnable = 0;
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
bool IsConnectionEstablished () {    // Determine if there is a program to talk to
  if (CommsEstablishedSerial == ESTABLISH_CONNECTION_SERIAL) {
    return true;
    } else {
      return false;
  }
}
// -------------------------------------------------------------------------------------------- //
bool IsProgramPaused () {
  if (PausedForAssistance = 1) {
    return true;
  } else {
    return false;
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

void readSerial () { // Generat a circular buffer to store incoming comamnds for later interpretation
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
bool TransTypeCommand () {
  if (CommandEnable == 1) {
    return true;
  } else {
    return false;
  }
}
// -------------------------------------------------------------------------------------------- //
bool TransTypeAcknowledge () {
  if (AcknowledgeEnable == 1) {
    return true;
  } else {
    return false;
  }
}
// -------------------------------------------------------------------------------------------- //
void processIncomingQueue () {   //interpret the byte pulled from the cue and execute the command
  // Pull off a single command from the queue if command has enough bytes.
  // For simplicity sake, all commands will be a total of 3 bytes (indicating command type, command id, command parameter)
  if (serialIncomingQueueFillAmt > 2) { // Having anything more than 2 means we have enough to pull a 3 byte command.
    // Check if first byte in queue indicates a command type (if not, throw it out and don't process more until next time processIncomingQueue is called)
    char byteRead = pullByteOffQueue();
    if (byteRead == TRANS_TYPE_COMMAND) {
      CommandEnable = 1;
      AcknowledgeEnable = 0;
    } else if (byteRead == TRANS_TYPE_ACKNOWLEDGE) {
      AcknowledgeEnable = 1;
      CommandEnable = 0;
    } else {
      byteRead = 0;
      CommandEnable = 0;
      AcknowledgeEnable = 0;
    } 

    if (TransTypeCommand() == true) {
      byteRead = pullByteOffQueue();
      switch(byteRead) {
        case ESTABLISH_CONNECTION:
          byteRead = pullByteOffQueue();
          if (byteRead = ESTABLISH_CONNECTION_SERIAL) {
            CommsEstablishedSerial = ESTABLISH_CONNECTION_SERIAL;
            WriteOnSerial(TRANS_TYPE_COMMAND);
            WriteOnSerial(CONNECTION_ESTABLISHED);
            WriteOnSerial(byteRead);
            Serial.print("\n");
          }
          break;

        case DISPENSE_CANDY:
          byteRead = pullByteOffQueue();
          WriteOnSerial(TRANS_TYPE_COMMAND);
          WriteOnSerial(DISPENSING_CANDY);
          ControlMotor(byteRead);
          WriteOnSerial(byteRead);
          Serial.print("\n");
        break;

        case RESET:
          byteRead = pullByteOffQueue();
          WriteOnSerial(TRANS_TYPE_COMMAND);
          WriteOnSerial(RESETTING);
          Restart();

      }
    } else if (TransTypeAcknowledge() == true) {
      byteRead = pullByteOffQueue();
      switch(byteRead) {
        case PAUSED_FOR_ASSISTANCE:
        byteRead = pullByteOffQueue();
          PausedForAssistance = 1;

    
    }

       //Make sure there is still a possible parameter byte
      if (serialIncomingQueueFillAmt > 0) {
        // Then see if byte matches command id; if not double bytes and do nothing until next time processIncomingQueue is called
          
      }
  }
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () { // Wait for a program to talk to
  while (IsConnectionEstablished() == false) {
    readSerial(); //listen on serial for the establish comms command from program
    processIncomingQueue(); //determine what was send over Serial
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //
