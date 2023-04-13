#include "hardware_operations.h" 
#include "communications.h"

#define ACKNOWLEDGE_COMMAND 0x55,0x54

bool WatchForCandyDispensed = false;
bool WatchForCandyTaken = false;

// -------------------------------------------------------------------------------------------- //
void setup() {

  // set up communications
  SetUpCommunications();
}
// -------------------------------------------------------------------------------------------- //
void loop() {
  // put your main code here, to run repeatedly:
  WriteOnSerial[ACKNOWLEDGE_COMMAND];
  }
// -------------------------------------------------------------------------------------------- //
