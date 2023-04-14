#include "hardware_operations.h" 
#include "communications.h"
// -------------------------------------------------------------------------------------------- //
#define COMMAND_ACKNOWLEGED 0x54 // Capital 'T'
#define CANDY_DISPENSED_RESPONSE 0x41 // Capital 'A'
#define CANDY_TAKEN_RESPONSE 0x4D // Capital 'M'
#define CANDY_JAM_OR_EMPTY_RESPONSE 0x59 // Capital 'Y'
#define MOTOR_ROTATION_PER_DISPENSE (MOTOR_ROTATION_FULL_360_DEG / 4)
// -------------------------------------------------------------------------------------------- //
bool WatchForCandyDispensed = false;
bool WatchForCandyTaken = false;
// -------------------------------------------------------------------------------------------- //
void setup() {
    // set up communications
  SetUpCommunications();
  
  // wait for software initialization
  EstablishConnectionToSoftware ();
  
  // set up hardware:
  SetUpHardware();
}
// -------------------------------------------------------------------------------------------- //
void loop() {
  // put your main code here, to run repeatedly:
  readSerial();
  processIncomingQueue();

  /* Old Code set to be replaced
  if (WatchForCandyDispensed && IsCandyDispensed ()) {
    WatchForCandyDispensed = false;
    WriteOnSerial(CANDY_DISPENSED_RESPONSE); 
    WatchForCandyTaken = true;
  }

  if (WatchForCandyTaken == true && IsCandyTaken()) {
    WatchForCandyTaken = false;
    WriteOnSerial(CANDY_TAKEN_RESPONSE); 
  }
  */
}
// -------------------------------------------------------------------------------------------- //
