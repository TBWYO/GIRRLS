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
DetermineCommTypes ();
}
// -------------------------------------------------------------------------------------------- //
