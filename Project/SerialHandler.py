import serial
import time
#Command Types
TRANS_TYPE_COMMAND = 0x7E
TRANS_TYPE_ACKNOWLEDGE = 0x40
TRANS_TYPE_EVENT = 0x25

#Host Parameters
ESTABLISH_CONNECTION_SERIAL = 0x53

#Host Commands
ESTABLISH_CONNECTION = 0x45
DISPENSE_CANDY = 0x49
RESET = 0x51

#Client Events
JAM_OR_EMPTY = 0xa
DISPENSE_DETECTED = 0x4d
CANDY_TAKEN = 0x54

#Host Acknowledgements
CONNECTION_ESTABLISHED = 0x65
DISPENSING_CANDY = 0x69
RESETTING = 0x71

#Client Acknowledgements
PAUSED_FOR_ASSISTANCE = 0x65
DISPENSE_NOTED = 0x6d
CANDY_TAKEN_NOTED = 0x74

#Hardware Events
MOTOR_ROTATE = 0x44

establishConnection = [TRANS_TYPE_COMMAND,ESTABLISH_CONNECTION,ESTABLISH_CONNECTION_SERIAL]
moveMotor = [TRANS_TYPE_COMMAND,DISPENSE_CANDY,MOTOR_ROTATE]
ser = serial.Serial("/dev/ttyUSB0", 9600, xonxoff = False)

def command_to_send(command, response):
    ser.reset_input_buffer()
    ser.write(command)
    if(ser.read(3) == response):
        return True;
    else:
        return False;
#print(ser.cts)
#print(ser.dtr)
time.sleep(3) # Temporary solution until flow control is implemented
if command_to_send(establishConnection,b"@es"):
    command_to_send(moveMotor,b"@iy")