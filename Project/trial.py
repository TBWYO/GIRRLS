#!/usr/bin/python
#Make sure to activate the virtual environment with .\env\scripts\activate
import os
import math
import numpy as np
import pandas as pd
import serial
import sys
import socket
from psychopy import core, data, event, gui, misc, visual, prefs, monitors
from itertools import count, takewhile
from typing import Iterator
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData
# Set the audio preferences to use PTB first and then import psychopy.sound
prefs.hardware['audioLib'] = ['PTB','pyo','pygame','sounddevice']
from psychopy import sound
import psychopy_sounddevice


# Set variables for paths we will be using, as well as other useful information
cwd = os.getcwd()
resourcePath = (cwd + r"\Resources")
dataPath = (cwd+r"\DataPath")
hostName = socket.gethostname()
monitor = "testMonitor"
default_values = {}
# Control keys
left_key = '1'
right_key = '4'
quit_key = 'q'

# Variables from the original project, need to figure out what everything actually does
num_disdaqs = 5 # Not sure what this is yet
TR = 3 # Shouldn't need not scanning
refresh = 16.7
initial_waittime = 5
stim_dur = 3
fdbk_dur = 5
disdaq_time = int(math.floor((num_disdaqs*3000)/refresh)) # 15s (5TRs) math.floor rounds to nearest int
num_trials = 60 # Per block.
trial_dur = 8 # On average.
lastHRF = 15 # Time in sec, adjusted on-fly to account for timing errors.
end_time = (TR * num_disdaqs) + (num_trials * trial_dur) + lastHRF

# These variables seem to relate to the choices
b0 = '0'
b1 = '1'
b2 = '2'
b3 = '3'
b4 = '4'


def settingsGUI():
    # Create a dictionary with default values for all the settings
    default_values = {
        "Participant": "",
        "Date": data.getDateStr(),
        "Computer": hostName,
        "Com Port": "",
        "Fullscreen": False,
        "Bluetooth": False,
        "Test": False,
    }
    # Create the GUI dialog
    dialog = gui.DlgFromDict(
        dictionary=default_values,
        title="Settings",
        order=["Participant", "Date", "Computer", "Com Port", "Fullscreen", "Bluetooth", "Test"],
        tip={
            "Name": "Enter the participant's name",
            "Com_Port": "Enter the communication port",
            "Computer": "Enter the computer name",
            "Date": "Enter the date",
            "Test": "Enter the test name",
            "Participant": "Enter the participant ID",
            "Fullscreen": "Enable/disable fullscreen mode",
            "Bluetooth": "Enable/disable Bluetooth",
            "Test?": "Enable/disable test mode",
        },
    )
    if dialog.OK:
        #print("User entered:", default_values)
        misc.toFile('lastParams.pickle', default_values)
    else:
        print("User cancelled the dialog")
        core.quit()
    return default_values
# Debug function to check variables and whatever else we need


settingsGUI();


def debug():
    print("Resource Path:", resourcePath)
    print("Data Path:", dataPath)
    print("hostName:", hostName)
    print("Sound Device:", sound.Sound.__name__)
    print("Bluetooth:", default_values['Bluetooth'])
# To run in debug mode run as python .\trial.py debug
if len(sys.argv) == 2 and sys.argv[1] == 'debug':
        debug();
