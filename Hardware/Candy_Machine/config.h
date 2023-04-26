#ifndef CONFIGURATIONS
#define CONFIGURATIONS

// Serial Paramaters
#define SERIAL_BAUD_RATE 9600

// ID10T Incoming Buffer Constants
#define SERIAL_INCOMING_BUFFER_SIZE 64 // If space in memory gets tight can reduce

// ID10T Outgoing Buffer Cosntants
#define SERIAL_OUTGOING_BUFFER_SIZE 64 // If space in memory gets tight can reduce

#define PIN_LED_FAIL   17
#define PIN_RESET 35
#define PIN_CANDY_DISPENSE_DETECT 27 // Beambreaker collector port
#define PIN_USER_EXTRACTION_DETECT 7 // Beambreaker collector port
#define MOTOR_PRIMARY_DISPENSE_SPEED 20 // 0 TO 255 Valid
#define MOTOR_ROTATION_PER_DISPENSE (MOTOR_ROTATION_FULL_360_DEG / 4)

#endif