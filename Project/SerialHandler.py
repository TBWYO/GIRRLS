import serial
import time
import threading
import keyboard
#Command Types
TRANS_TYPE_COMMAND = 0x7E
TRANS_TYPE_ACKNOWLEDGE = 0x40
TRANS_TYPE_EVENT = 0x25

ser = serial.Serial('/dev/ttyUSB0', 9600)
ser.flushInput()
ser.flushOutput()
time.sleep(3)
ser.write(b'~ES')
ser.flush()

expected_response =b'@es'
response = b''

while response != expected_response:
    if ser.inWaiting() > 0:
        response += ser.read()

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

ser = serial.Serial("COM4", 9600, xonxoff = True)

def command_to_send(command):
    #print("Running Command to Send")
    ser.reset_input_buffer()
    ser.write(command)
    return True;

def read_from_port(serial):
    #print("Read from port thread started")
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
thread = threading.Thread(target=read_from_port,args=(ser,))
thread.start()

time.sleep(2)
command_to_send(establishConnection)
while reading_serial:
    if keyboard.read_key() == "d":
        command_to_send(moveMotor)
    if keyboard.read_key() == "x":
        reading_serial = False
        