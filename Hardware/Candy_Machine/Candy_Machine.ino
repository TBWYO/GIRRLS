#include "hardware_operations.h" 
#include "communications.h"
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
    DetermineCommTypes (); // listen on serial
    }
  Restart();
}
// -------------------------------------------------------------------------------------------- //
