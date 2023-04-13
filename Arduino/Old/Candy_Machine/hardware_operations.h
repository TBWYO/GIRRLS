#ifndef HARDWARE_OPERATIONS
#define HARDWARE_OPERATIONS

#define MOTOR_ROTATION_FULL_360_DEG 516

void SetFailLed(bool setLedOn);
void SetUpHardware ();
void MotorMovePrimaryDispense (int StepsToMove);
bool IsCandyDispensed (); 
bool IsCandyTaken ();
#endif
