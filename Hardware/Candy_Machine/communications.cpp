#include <Arduino.h>
#include "hardware_operations.h"
#include "communications.h"
// -------------------------------------------------------------------------------------------- //
// Serial Paramaters
#define SERIAL_BAUD_RATE 9600
// ID10T TRANS Types
#define TRANS_TYPE_COMMAND 0x7E // The command trans type is denoted by a Tilde '~'
#define TRANS_TYPE_ACKNOWLEDGE 0x40 //The acknowledge trans type is denoted by an "AT" symbol '@'
#define TRANS_TYPE_EVENT 0x25 // %
bool TransTypeCommand = false; 
bool TransTypeAcknowledge = false;
bool TransTypeEvent = false;
// ID10T Host Commands
int ESTABLISH_CONNECTION[3] = {0x7e,0x45,0x53}; 
int DISPENSE_CANDY[3] = {0x7e,0x49,0x44}; // This command is denoted by a capital 'I' 
#define RESET 0x51 // This command is denoted by a capital 'Q'
// ID10T Host Acknowledgements
char ESTABLISH_CONNECTION_SERIAL_RESPONSE[3] = {0x40,0x65,0x73};
char MOTOR_ROTATE_RESPONSE[3] = {0x40,0x69,0x79};
// ID10T Incoming Buffer Constants
#define SERIAL_INCOMING_BUFFER_SIZE 64
// ID10T Incoming Buffer Integers
char SerialIncomingQueue[SERIAL_INCOMING_BUFFER_SIZE];
int SerialIncomingQueueFillAmt = 0; // How much is available to read
int serialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int SerialIncomingWritePointer = 0; // Index where to write next byte
bool ResetToggle = false;
bool IsConnectionEstablished = false;
bool IsProgramPaused = false; 
bool WaitingForCommand = false;
// ID10T Outgoing Buffer Cosntants
#define SERIAL_OUTGOING_BUFFER_SIZE 64
// ID10T Outgoing Buffer Integers
char serialOutgoingQueue[SERIAL_OUTGOING_BUFFER_SIZE];
int SerialOutgoingQueueFillAmt = 0; // How much is available to read
int SerialOutgoingReadPointer = 0; // Index in queue to start read (circular buffer)
int SerialOutgoingWritePointer = 0; // Index where to write next byte
int BytesToWrite[3] = {0,0,0};
bool WatchForCandyDispensed = false;
bool WatchForCandyTaken = false;
bool WaitingForCommand = false;
// -------------------------------------------------------------------------------------------- //
void SetUpCommunications() {
  // Set up Serial at predefined baudrate
  Serial.begin(SERIAL_BAUD_RATE);
  // Wait for Serial           
  while (!Serial) {
    SetFailLed(true);
  }
  SetFailLed(false);
  }
// -------------------------------------------------------------------------------------------- //
int ListenOnSerial () {   // Listen on Serial fallback for Buffer
  int IncomingByte = 0;
  if (Serial.available() > 0) {
    // read the incoming byte:
    IncomingByte = Serial.read();
    // Serial.println(IncomingByte, HEX); // For debug only
  }
  return IncomingByte;
  }
// -------------------------------------------------------------------------------------------- //
void WriteArrayOnSerial (char* SendOnSerialArray, int length) {   // Output to the Serial BUS
  Serial.write(SendOnSerialArray, length);
  }
// -------------------------------------------------------------------------------------------- //
void ReadSerial () { // Generat a circular buffer to store incoming comamnds for later interpretation
  int BytesToRead = Serial.available();
  if (BytesToRead > 0) {
    // Dump each byte to queue
    for (int i = 0; i < BytesToRead; i++) {
      // Check to be sure room still exists in buffer
      if (SerialIncomingQueueFillAmt < SERIAL_INCOMING_BUFFER_SIZE) {
        // int SerialReadByte = Serial.read();
        // if (SerialReadByte ==XON)
        SerialIncomingQueue[SerialIncomingWritePointer] = Serial.read();
        // Serial.println(SerialIncomingQueue[SerialIncomingWritePointer], HEX); // For debug only
        // Increase count of what is in buffer
        SerialIncomingQueueFillAmt++;

        // Move where to write next byte (may need to loop back to start of array if end is full).
        // Note that end is based on a start index of 0 so 64 size would have index range is 0-63
        if (SerialIncomingWritePointer == SERIAL_INCOMING_BUFFER_SIZE - 1) {
          // Loop back to first index
          SerialIncomingWritePointer = 0;
        } else {
          SerialIncomingWritePointer++;
        }
      }
    }
  }
  }
// -------------------------------------------------------------------------------------------- //
char PullByteOffIncomingQueue () {   // read the bytes stored in the incoming buffer
  // Note that this function is not verifying bytes exist before pulling so you MUST be sure there is a usable byte BEFORE calling this function
  char returnValue = SerialIncomingQueue[serialIncomingReadPointer];
  // consume up the byte read by moving the read pointer and decreasing fill amount
  if (serialIncomingReadPointer == SERIAL_INCOMING_BUFFER_SIZE - 1) {
    // Loop back to first index
    serialIncomingReadPointer = 0;
  } else {
    serialIncomingReadPointer++;
  }
  SerialIncomingQueueFillAmt--;
  return returnValue;
  }
// -------------------------------------------------------------------------------------------- //
void ProcessIncomingQueue () {   //interpret the byte pulled from the cue and execute the command
  // Pull off a single command from the queue if command has enough bytes.
  // For simplicity sake, all commands will be a total of 3 bytes (indicating command type, command id, command parameter)
  if (SerialIncomingQueueFillAmt > 2) { // Having anything more than 2 means we have enough to pull a 3 byte command.
    // Check if first byte in queue indicates a command type (if not, throw it out and don't process more until next time ProcessIncomingQueue is called)
    char ByteRead = PullByteOffIncomingQueue();
     if (ByteRead == TRANS_TYPE_COMMAND) {
      TransTypeCommand = true;
      TransTypeAcknowledge = false;
    } else if (ByteRead == TRANS_TYPE_ACKNOWLEDGE) {
      TransTypeAcknowledge = true;
      TransTypeCommand = false;
    } else {
      ByteRead = 0;
      TransTypeCommand = false;
      TransTypeAcknowledge = false;
    }
    if (TransTypeCommand) { 
      ByteRead = PullByteOffIncomingQueue();
      if (ByteRead == ESTABLISH_CONNECTION[1]) {
        ByteRead = PullByteOffIncomingQueue();
        if (ByteRead == ESTABLISH_CONNECTION[2]) {
          IsConnectionEstablished = true;
          BytesToWrite[3] = {ESTABLISH_CONNECTION_SERIAL_RESPONSE}; 
          void WriteOutgoingBuffer ();
        }
      } else if (ByteRead == DISPENSE_CANDY[1]) {
        ByteRead = PullByteOffIncomingQueue();
          if (ByteRead == DISPENSE_CANDY[2]) {
          ControlMotor(ByteRead);
          WatchForCandyDispensed = true;
          BytesToWrite[3] = {MOTOR_ROTATE_RESPONSE};
          WriteOutgoingBuffer (); 
          }
      } /*else if (ByteRead == RESET) {
        ByteRead = PullByteOffIncomingQueue();
        //WriteOnSerial(TRANS_TYPE_COMMAND);
        //WriteOnSerial(RESETTING);
        Restart();
      } */
       //Make sure there is still a possible parameter byte
      if (SerialIncomingQueueFillAmt > 0) {
        // Then see if byte matches command id; if not double bytes and do nothing until next time ProcessIncomingQueue is called          
      }
  }
  }
  }
// -------------------------------------------------------------------------------------------- //
void WriteOutgoingBuffer () {
  if (BytesToWrite[3] > {0,0,0}) {  //determine if something at all has been set to write to the buffer
    for (int i - 0; i < BytesToWrite; i++) {
      if (SerialOutgoingQueueFillAmt < SERIAL_OUTGOING_BUFFER_SIZE) {
        serialOutgoingQueue[SerialOutgoingWritePointer] = Serial.read();
        SerialOutgoingQueueFillAmt++;
        if (SerialOutgoingWritePointer == SERIAL_OUTGOING_BUFFER_SIZE -1) {
          SerialOutgoingWritePointer = 0;
        } else {
          SerialOutgoingWritePointer++;
        }
      }
    }
  }
  }
// -------------------------------------------------------------------------------------------- //
char PullByteOffOutgoingQueue () {   // read the bytes on the buffer
  // Note that this function is not verifying bytes exist before pulling so you MUST be sure there is a usable byte BEFORE calling this function
  char returnValue = SerialOutgoingQueue[serialOutgoingReadPointer];

  // consume up the byte read by moving the read pointer and decreasing fill amount
  if (SerialOutgoingReadPointer == SERIAL_OUTGOING_BUFFER_SIZE - 1) {
    // Loop back to first index
    serialOutgoingReadPointer = 0;
  } else {
    serialOutgoingReadPointer++;
  }
  SerialOutgoingQueueFillAmt--;

  return returnValue;
}
// -------------------------------------------------------------------------------------------- //
void ProcessOutgoingQueue () { // pull the bytes off of the outgoing buffer, analyze them, reconstruct them, then send the array over serial
  if (SerialOutgoingQueueFillAmt > 2) {
    char ByteFromQueue1 = PullByteOffOutgoingQueue ();
    if (ByteFromQueue1 == TRANS_TYPE_COMMAND) {
      TransTypeCommand = true;
      TransTypeAcknowledge = false;
      TransTypeEvent = false;
      char ByteToWrite1 = ByteFromQueue1;
    } else if (ByteFromQueue1 == TRANS_TYPE_ACKNOWLEDGE) {
        TransTypeCommand = false;
        TransTypeAcknowledge = true;
        TransTypeEvent = false;
        char ByteToWrite1 = ByteFromQueue1;
    } else if (ByteFromQueue1 == TRANS_TYPE_ACKNOWLEDGE) {
        TransTypeCommand = false;
        TransTypeAcknowledge = false;
        TransTypeEvent = true;
        char ByteToWrite1 = ByteFromQueue1;
    } else {
        TransTypeCommand = false;
        TransTypeAcknowledge = false;
        TransTypeEvent = false;
        ByteFromQueue1 = 0;
    }
    if (TransTypeCommand) {
      // To send a command to the python game that is sent over the outgoing buffer, add it here
    } else if (TransTypeAcknowledge) {
      char ByteFromQueue2 = PullByteOffOutgoingQueue ();
      if (ByteFromQueue2 == 0x65) { // determine if the acknoledgement is a connection established command
        char ByteToWrite2 = ByteFromQueue2;
        char ByteFromQueue3 = PullByteOffOutgoingQueue ();
        if (ByteFromQueue3 == 0x73) { // determine if serial is used
          char BytesToWrite3 = ByteFromQueue3;
        } else if (ByteFromQueue3 == 0x62) { // determine if bluetooth is used
          char BytesToWrite3 = ByteFromQueue3;
        }
      } else if (ByteFromQueue2 = 0x49) {
        char ByteToWrite2 = ByteFromQueue2;
        char ByteFromQueue3 = PullByteOffOutgoingQueue ();
        if (ByteFromQueue3 == 0x79) { // determine if the state was a sucess 
          char BytesToWrite3 = ByteFromQueue3;
        } else if (ByteFromQueue3 == 0x7a) { //determine if the state was a fail
          char BytesToWrite3 = ByteFromQueue3;
        }
      }
    } else if (TransTypeEvent) {
      char ByteFromQueue2 = PullByteOffOutgoingQueue ();
      if (ByteFromQueue1 == 0x4a) { // determine if the event is a jam or empty event (cannot tell the difference)
        char ByteToWrite2 = ByteFromQueue2;
        char ByteFromQueue3 = PullByteOffOutgoingQueue ();
        if (ByteFromQueue3 == 0x50) { //determine if the parameter is a pause (always will be)
          char BytesToWrite3 = ByteFromQueue3;
    }
  } else if (ByteFromQueue1 == 0x4d) { // determine if the event is a dispense detected
        char ByteToWrite2 = ByteFromQueue2;
        char ByteFromQueue3 = PullByteOffOutgoingQueue ();
        if (ByteFromQueue3 == 0x43) { // determine if the python game should increase the total count of candies (it thinks) are in the tray
          char BytesToWrite3 = ByteFromQueue3;
        }
  } else if (ByteFromQueue1 == 0x54) { // determine if the event was a candy been taken
          char ByteToWrite2 = ByteFromQueue2;
          char ByteFromQueue3 = PullByteOffOutgoingQueue ();
          if (ByteFromQueue3 == 0x52) { // determine if the python game should decrease the total count of candies (it thinks) are in the tray
            char BytesToWrite3 = ByteFromQueue3;
          }
    int BytesToSend[3] = {ByteFromQueue1,ByteToWrite2,ByteFromQueue3};
    WriteArrayOnSerial(BytesToSend, 3);
  }
  }
    }
  }
// -------------------------------------------------------------------------------------------- //
void DetermineCommTypes () {
   
  if (WaitingForCommand) {
    readSerial();
    ProcessIncomingQueue();
  } else if (WatchForCandyDispensed && IsCandyDispensed ()) {
    WatchForCandyDispensed = false;
    WatchForCandyTaken = true;
    BytesToWrite[3] = {0x25,0x4d,0x43} // write the bytes needed to indicate dispense to the outgoing buffer (very temporary)
    WriteOutgoingBuffer ();
  } else if (WatchForCandyTaken == true && IsCandyTaken()) {
    WatchForCandyTaken = false;
    BytesToWrite[3] = {0x25,0x54,0x52} // write the bytes needed to indicate dispense to the outgoing buffer (very temporary)
    WriteOutgoingBuffer ();
  }
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () { 
  while (!IsConnectionEstablished) {
    readSerial(); 
    ProcessIncomingQueue(); 
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //