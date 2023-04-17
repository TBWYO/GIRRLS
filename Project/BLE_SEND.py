import serial
ser = serial.Serial("/dev/ttyUSB0", 9600, write_timeout = 3)
ser.write(b'~ES')  
ser.write(b'~ID')  
ser.close