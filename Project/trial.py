#!/usr/bin/python

#Python -m psychopy.monitors.MonitorCenter
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
# Set the audio preferences to use PTB first, sound and psychopy_sounddevice must be imported after 
prefs.hardware['audioLib'] = ['PTB','pyo','pygame','sounddevice']
from psychopy import sound
import psychopy_sounddevice


# Set variables for paths we will be using
cwd = os.getcwd()
resourcePath = (cwd + r"\Resources")
dataPath = (cwd+r"\DataPath")
hostname = socket.gethostname()

def debug():
    print("Resource Path:", resourcePath)
    print("Data Path:", dataPath)
    print("Hostname:", hostname)
    print("Sound Device:", sound.Sound.__name__)

def settingsUI():
    monSize = [1920, 1080]
    info = {}
    info['FullScreen'] = False
    info['Test?'] = False
    info['1. Participant'] = '[PARTICIPANT]'
    info['Computer'] = hostname
    info['Date'] = data.getDateStr()
    info['Bluetooth'] = True
    info['Com_port'] = 'COM4'
    dlg = gui.DlgFromDict(info)
    if dlg.OK:
        misc.toFile('PST_fMRI_lastParams.pickle', info) 
    else:
        core.quit()
settingsUI();
debug();