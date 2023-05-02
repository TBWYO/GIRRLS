import serial
import time
import threading
import keyboard

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
reading_serial = True

establishConnection = [TRANS_TYPE_COMMAND,ESTABLISH_CONNECTION,ESTABLISH_CONNECTION_SERIAL]
moveMotor = [TRANS_TYPE_COMMAND,DISPENSE_CANDY,MOTOR_ROTATE]



def connect_serial(comport,baudrate):
    global ser;
    global reading_serial;
    ser = serial.Serial(comport, baudrate, xonxoff = True)
    time.sleep(3)
    thread = threading.Thread(target=read_from_port)
    thread.start()
    print("Connected to serial")
    reading_serial = True;
    return ser, reading_serial;

def command_to_send(command):
    #print("Running Command to Send")
    ser.reset_input_buffer()
    ser.write(command)
    return True;

def read_from_port():
    print("Read from port thread started")
    while reading_serial:
        if ser.in_waiting >= 3:
            bytes_read = ser.read(3)
            if bytes_read == b"@es":
                print("Connection Established")
            elif bytes_read == b"@iy":
                print("Motor Moved")
            elif bytes_read == b"%MC":
                print("Candy Dispensed")
            elif bytes_read == b"%TR":
                print("Candy Taken")
            else:
                print(bytes_read)