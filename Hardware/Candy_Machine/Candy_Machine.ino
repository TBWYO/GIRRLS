#include "config.h"

#include "hardware_operations.h" 
#include "communications.h"

// Events set to PC
char EVENT_CANDY_DISPENSED[3] = {0x25,0x4d,0x43};
char EVENT_CANDY_TAKEN[3] = {0x25,0x54,0x52};

// -------------------------------------------------------------------------------------------- //
void setup() {
  //Determine if the python program is present
  EstablishConnectionToSoftware ();
  // set up hardware:
  SetUpHardware();
}
// -------------------------------------------------------------------------------------------- //
void WatchBeamBreakers () {
  // Candy Dispensed
  if (getWatchForCandyDispensed() && IsCandyDispensed ()) {
    setWatchForCandyDispensed(false);
    setWatchForCandyTaken(true);
    WriteOutgoingBuffer (EVENT_CANDY_DISPENSED, sizeof(EVENT_CANDY_DISPENSED));
  }

  // Candy Taken
  if (getWatchForCandyTaken() && IsCandyTaken()) {
    setWatchForCandyTaken(false);
    WriteOutgoingBuffer (EVENT_CANDY_TAKEN, sizeof(EVENT_CANDY_TAKEN));
  }
}
// -------------------------------------------------------------------------------------------- //
void loop() {
  DetermineCommTypes ();
  WatchBeamBreakers();
}
// -------------------------------------------------------------------------------------------- //
