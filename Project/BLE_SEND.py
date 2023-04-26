import serial
ser = serial.Serial("/dev/ttyUSB0", 9600, write_timeout = 3)
ser.write(b'~ES')
print (ser.read(3))
ser.write(b'~ID')
print (ser.read(3))
