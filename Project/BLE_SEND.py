import asyncio
import logging
import sys
from os.path import exists
from bleak import BleakClient

file_name = sys.argv[1] #the variable to store the name of the txt file (which stores beam break timecount, for details see the readme txt file)
#the name of the txt file is set during the launch of the BLE_SEND script


#below are the parameters of bluetooth
#these params are the same for every feather board
UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


#below starts the asyncronous part of script
#this is the main body of the script


if exists(file_name+'.txt'): #if there's a file with the name we assigned above (when we launch the ble_send)
            #open this file, input the data and close it
    f = open(file_name+'.txt', 'a') #'a' stands for append mode (not just write bcs write would overwrite the data)               
else: #if the file doesn't exist, we create it and write the data in it
    f = open(file_name+'.txt', 'w')
                

#we're creating the function

async def run(address):


#creating another function
    #this function receives data from the hardware arduino script
    async def handle_rx(_: int, data: bytearray): # we dunno what's the _: part
        #data is the var for data received from Arduino #bytearray is the data type
        recieved_time = int(data) #var to store the received data as integer
        print("Time to break: "+str(recieved_time)) #control msg (checkpoint) to be printed in the terminal
        f.write(str(recieved_time)+' \n')
        f.close()
        for task in asyncio.all_tasks(): #this part secures closure of the script without errors 
            task.cancel()
        sys.exit(0) #this line closes the script (despite the while true function)

#this part below operates the motor

    async with BleakClient(address) as client:
        paired = await client.pair() #connects the bluetooth of the PC to the feather board
        await client.start_notify(UART_RX_CHAR_UUID.lower(), handle_rx) #this line makes the script wait for receiving the data from arduino
        #and then launch the handle_rx function 

        text = 'T' #T stands for True, the T is sent to Arduino script to launch the motor
        #the arduino script is set so that the motor launches when the T signal is received
        await client.write_gatt_char(UART_TX_CHAR_UUID.lower(), bytes(text, encoding = 'ascii')) #this function send the T to the arduino script

#the part below makes the script wait for receiving data from arduino and not close before the data is received

        while True: #if we don't have this line, BleakClient would close before the data is received
            #if Bleakclient closes, we'll lose the bluetooth connection
            await asyncio.sleep(1) #this line ensures that all the await functions do not wait for the while true to be executed
            #while True is the infinite loop (makes the script run forever)
        

      
if __name__ == "__main__":
    #this is the feather bluetooth parameters
    #this parameter is unique for every feather board
    #we found this param using the scan function from the bluez library 
    address = "C4:B9:DA:5F:83:50"

    #these lines below launch the async script
    loop = asyncio.get_event_loop()
    loop.set_debug(True)
    loop.run_until_complete(run(address))
    
