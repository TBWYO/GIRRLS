import serial
import time

ser = serial.Serial('/dev/tyUSB0', 9600)
ser.flushInput()
ser.flushOutput()

ser.write(b'~ES')
ser.flush()

expected_response =b'@es'
response = b''

while response != expected_response:
    if ser.inWaiting() > 0:
        response += ser.read():

ser.close()
print(response)

