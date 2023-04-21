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
int SerialIncomingReadPointer = 0; // Index in queue to start read (circular buffer)
int SerialIncomingWritePointer = 0; // Index where to write next byte
bool ResetToggle = false;
bool IsConnectionEstablished = false;
bool IsProgramPaused = false; 
// ID10T Outgoing Buffer Cosntants
#define SERIAL_OUTGOING_BUFFER_SIZE 64
#define CHECK_IF_ENOUGH_BYTES_TO_WRITE_TO_QUEUE 3
// ID10T Outgoing Buffer Integers
char SerialOutgoingQueue[SERIAL_OUTGOING_BUFFER_SIZE];
int SerialOutgoingQueueFillAmt = 0; // How much is available to read
int SerialOutgoingReadPointer = 0; // Index in queue to start read (circular buffer)
int SerialOutgoingWritePointer = 0; // Index where to write next byte
bool WatchForCandyDispensed = false;
bool WatchForCandyTaken = false;
bool ReadyToWrite = false;
bool WaitingForCommand = true;
// -------------------------------------------------------------------------------------------- //
void setWatchForCandyDispensed (bool newValue) {
  WatchForCandyDispensed = newValue;
}
// -------------------------------------------------------------------------------------------- //
bool getWatchForCandyDispensed () {
  return WatchForCandyDispensed;
}
// -------------------------------------------------------------------------------------------- //
void setWatchForCandyTaken (bool newValue) {
  WatchForCandyTaken = newValue;
}
// -------------------------------------------------------------------------------------------- //
bool getWatchForCandyTaken () {
  return WatchForCandyTaken;
}
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
void WriteArrayOnSerial (char* SendOnSerialArray, int length) {   // Output to the Serial BUS
  Serial.write(SendOnSerialArray, length);
  }
// -------------------------------------------------------------------------------------------- //
void ReadSerial () { // Generat a circular buffer to store incoming comamnds for later interpretation
  int BytesToRead = Serial.available();
  if (BytesToRead > 0) {
    // Dump each b{0x25,0x54,0x52}yte to queue
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
  char returnValue = SerialIncomingQueue[SerialIncomingReadPointer];
  // consume up the byte read by moving the read pointer and decreasing fill amount
  if (SerialIncomingReadPointer == SERIAL_INCOMING_BUFFER_SIZE - 1) {
    // Loop back to first index
    SerialIncomingReadPointer = 0;
  } else {
    SerialIncomingReadPointer++;
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
          // char* BytesToWrite[3] = {ESTABLISH_CONNECTION_SERIAL_RESPONSE}; 
          WriteOutgoingBuffer (ESTABLISH_CONNECTION_SERIAL_RESPONSE, sizeof(ESTABLISH_CONNECTION_SERIAL_RESPONSE));
        }
      } else if (ByteRead == DISPENSE_CANDY[1]) {
        ByteRead = PullByteOffIncomingQueue();
          if (ByteRead == DISPENSE_CANDY[2]) {
          ControlMotor(ByteRead);
          WatchForCandyDispensed = true;
          // char* BytesToWrite[3] = {MOTOR_ROTATE_RESPONSE};
          WriteOutgoingBuffer (MOTOR_ROTATE_RESPONSE, sizeof(MOTOR_ROTATE_RESPONSE)); 
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
void WriteOutgoingBuffer (char* ByteArray, int length) {
  //Serial.write('1');
  if (length >= CHECK_IF_ENOUGH_BYTES_TO_WRITE_TO_QUEUE) { 
    //Serial.write('2');
    for (int i = 0; i < length; i++) {
      // Check to be sure room still exists in buffer
      if (SerialOutgoingQueueFillAmt < SERIAL_OUTGOING_BUFFER_SIZE) {
        //Serial.write('3');
        // int SerialReadByte = Serial.read();
        // if (SerialReadByte ==XON)
        SerialOutgoingQueue[SerialOutgoingWritePointer] = ByteArray[i];
        // Serial.println(SerialIncomingQueue[SerialIncomingWritePointer], HEX); // For debug only
        // Increase count of what is in buffer
        SerialOutgoingQueueFillAmt++;
        // Move where to write next byte (may need to loop back to start of array if end is full).
        // Note that end is based on a start index of 0 so 64 size would have index range is 0-63
        if (SerialOutgoingWritePointer == SERIAL_OUTGOING_BUFFER_SIZE - 1) {
          //Serial.write('4');
          // Loop back to first index
          SerialOutgoingWritePointer = 0;
        } else {
          //Serial.write('5');
          SerialOutgoingWritePointer++;
        }
      }
    }
  }
  //WriteArrayOnSerial (SerialOutgoingQueue, 3);
}
// -------------------------------------------------------------------------------------------- //
char PullByteOffOutgoingQueue () {   // read the bytes on the buffer
  // Note that this function is not verifying bytes exist before pulling so you MUST be sure there is a usable byte BEFORE calling this function
  char returnValue = SerialOutgoingQueue[SerialOutgoingReadPointer];

  // consume up the byte read by moving the read pointer and decreasing fill amount
  if (SerialOutgoingReadPointer == SERIAL_OUTGOING_BUFFER_SIZE - 1) {
    // Loop back to first index
    SerialOutgoingReadPointer = 0;
  } else {
    SerialOutgoingReadPointer++;
  }
  SerialOutgoingQueueFillAmt--;
  return returnValue;
}
// -------------------------------------------------------------------------------------------- //
void ProcessOutgoingQueue () { // pull the bytes off of the outgoing buffer, analyze them, reconstruct them, then send the array over serial
  if (SerialOutgoingQueueFillAmt >= CHECK_IF_ENOUGH_BYTES_TO_WRITE_TO_QUEUE) {
    char ByteToWrite1 = PullByteOffOutgoingQueue();
    char ByteToWrite2 = PullByteOffOutgoingQueue();
    char ByteToWrite3 = PullByteOffOutgoingQueue();
    char BytesToSend[3] = {ByteToWrite1,ByteToWrite2,ByteToWrite3};
    WriteArrayOnSerial(BytesToSend, sizeof(BytesToSend));
  }
}
// -------------------------------------------------------------------------------------------- //
void DetermineCommTypes () {
  ReadSerial();
  ProcessIncomingQueue();
  ProcessOutgoingQueue ();
}
// -------------------------------------------------------------------------------------------- //
void EstablishConnectionToSoftware () { 
  while (!IsConnectionEstablished) {
    DetermineCommTypes();
    SetFailLed(true);
  }
  SetFailLed(false);
}
// -------------------------------------------------------------------------------------------- //