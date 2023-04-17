
# Candy Machine Probability Project 


The GIRRLS Project is a collaboration between Central Wyoming College and University of Wyoming. The project consists of three (3) parts, 
1) The Python Game 
2) The arduino electronics
3) The 3D printable chasis

### Briefly descrition of how it should work:

1) A child plays a game coded in Python.
2) When a child wins and thus launches the reward response, the Python will send a signal to the arduino device and activate the motor.
3) When a child places a hand between the first pair of sensors, the hand will break the infrared beam which will stop the motor.

### Python Game

To run the probability task, you will need to have Python 3.8.10 installed on your computer. You can download Python 3.8.10 from the official website and install the required libraries using the following command from the (project's) root directory:
```
pip install -r requirements.txt
```

### Arduino Electronics

To flash your nrf52832 with our code you need to have the arduino IDE installed with the proper board manager and COM port set.

Download the 1.8.2 version of the Arduino Integrated Development Environment (IDE) software.

Open the Arduino IDE and go to the "Preferences" menu.

Add a new URL for the Adafruit board manager by adding the following link to the "Additional Board Manager URLs" setting: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json.

Go to the "Boards Manager" under the "Tools" menu, search for "Adafruit nRF52," and install it.
Select the "Adafruit Feather nRF52832" board from the "Tools" menu.

Connect your board to your computer.

Identify the correct serial port for your board in the "Device Manager."

Make sure the correct serial port is selected in the Arduino IDE under the "Tools" menu.

Download the code file from the following link: https://drive.google.com/file/d/1c-6pmIpI3OBxZbIts5aktu7tMJwVAPpF/view?usp=share_link

Create a folder for the code and open it in the Arduino IDE.

Compile the code, but you will encounter error messages because some libraries are missing.

Download and install the "Adafruit Motorshield.h" and "Adafruit_I2CDevice.h" libraries from the internet.

Recompile the code and upload it to the board.

Test the code to ensure it's working properly.

That's it! The steps may seem complicated, but with persistence and some patience, you can get your board up and running in no time.

### 3D Printable Chasis

More information soon to come
