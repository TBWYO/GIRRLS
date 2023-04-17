#include "hardware_operations.h" 
#include "communications.h"
// -------------------------------------------------------------------------------------------- //
bool WatchForCandyDispensed = false;
bool WatchForCandyTaken = false;
// -------------------------------------------------------------------------------------------- //
void setup() {
    // set up communications
  SetUpCommunications();
    //Determine if the python program is present
  EstablishConnectionToSoftware ();
  // set up hardware:
  SetUpHardware();
}
// -------------------------------------------------------------------------------------------- //

void loop() {
  // put your main code here, to run repeatedly:
  while (ResetToggle() == false) { // determine if the reset command has been triggered
    readSerial(); // check the Serial bus for a 3 byte command
    processIncomingQueue(); // determine the command that was sent and execute it

  
  if (WatchForCandyDispensed && IsCandyDispensed ()) {
    WatchForCandyDispensed = false;
    WriteOnSerial(CANDY_DISPENSED_RESPONSE); 
    WatchForCandyTaken = true;
  }

  if (WatchForCandyTaken == true && IsCandyTaken()) {
    WatchForCandyTaken = false;
    WriteOnSerial(CANDY_TAKEN_RESPONSE); 
  }

}
  Restart();
}
// -------------------------------------------------------------------------------------------- //
