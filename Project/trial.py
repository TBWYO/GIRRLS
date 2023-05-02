#!/usr/bin/python
#Make sure to activate the virtual environment with .\env\scripts\activate
# Requires Python 3.8.10
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
import SerialHandler
import keyboard


# Set variables for paths we will be using, as well as other useful default_valuesrmation
CWD = os.getcwd()
RESOURCE_PATH = (CWD + r"\Resources")
DATA_PATH = (CWD+r"\Datapath")
HOST_NAME = socket.gethostname()
TEST_MONITOR = "testMonitor"
# Control keys
LEFT_KEY = '1'
RIGHT_KEY = '4'
QUIT_KEY = 'q'
#Test Keys
SIMULATE_CORRECT = "c"
SIMULATE_INCORRECT = "v"
START_MOTOR = "b"
STOP_MOTOR = "n"

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
baud = 9600
# These variables seem to relate to the choices
b0 = '0'
b1 = '1'
b2 = '2'
b3 = '3'
b4 = '4'

def settingsGUI():
    print("Showing GUI")
    # Create a dictionary with default values for all the settings
    default_values = {
        "Participant": "ParticipantName",
        "Date": data.getDateStr(),
        "Computer": HOST_NAME,
        "Com Port": "COM4",
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
            "Com Port": "Enter the communication port",
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
        return default_values

    else:
        print("User cancelled the dialog")
        core.quit()
default_values = settingsGUI();

# Debug function to check variables and whatever else we need
def debug():
    print("="*50)
    print("{:^50}".format("DEBUG STATEMENT"))
    print("="*50)
    print("{:<15}: {}".format("Resource Path", RESOURCE_PATH))
    print("{:<15}: {}".format("Data Path", DATA_PATH))
    print("{:<15}: {}".format("Host Name", HOST_NAME))
    print("{:<15}: {}".format("Sound class", sound.Sound.__name__))
    print("{:<15}: {}".format("Computer", default_values['Computer']))
    print("{:<15}: {}".format("Participant", default_values['Participant']))
    print("{:<15}: {}".format("Fullscreen", default_values['Fullscreen']))
    print("{:<15}: {}".format("Test", default_values['Test']))
    print("{:<15}: {}".format("Bluetooth", default_values['Bluetooth']))
    print("{:<15}: {}".format("Com Port", default_values['Com Port']))
# To run in debug mode run as python .\trial.py debug
if len(sys.argv) == 2 and sys.argv[1] == 'debug':
        debug();


if default_values['Bluetooth'] == False:
    
    SerialHandler.connect_serial(default_values['Com Port'],baud)
    SerialHandler.command_to_send(SerialHandler.establishConnection)    

if default_values['Test']:
    while SerialHandler.reading_serial:
        if keyboard.read_key() == "d":
            SerialHandler.command_to_send(SerialHandler.moveMotor)
        if keyboard.read_key() == "x":
            SerialHandler.reading_serial = False

##END OF CHANGES 

def check_rand (in_array,num_array,num_row): #Cannot have more than 6 consecutive reward outcomes scheduled.
    counter = 0
    for x in range(num_array):
        for y in range(num_row):
            if in_array[x,y,2] == 1:
                counter += 1
                if counter == 6:
                    return False
            else:
                counter = 0
    return True

def show_resp(action,l_stim_num,r_stim_num,frames,measured_refresh,start_time):
    refresh = measured_refresh
    resp_onset = start_time
    
    if action == 'left':
        while frames < int(math.floor(3000/refresh)):
            left_stim.draw()
            right_stim.draw()
            fix.draw()
            left_choice.draw()
            win.flip()
            frames = frames + 1

        if l_stim_num < r_stim_num: 
            accuracy = 1
            
        else: 
            accuracy = 0
            
    if action == 'right':
        while frames < int(math.floor(3000/refresh)):
            left_stim.draw()
            right_stim.draw()
            fix.draw()
            right_choice.draw()
            win.flip()
            frames = frames + 1

        if l_stim_num > r_stim_num: 
            accuracy = 1
            
        else: 
            accuracy = 0

    return (accuracy, resp_onset)

def show_fdbk(accuracy,sched_out,action,start_time,measured_refresh):
    refresh = measured_refresh
    fdbk_clock = core.Clock()
    fdbk_clock.reset()
    fdbk_onset = start_time
    #ble_launch_string is the var for launching the ble_send.py
    #this tells that ble_send will only run if
    #a) when we launch the ble_send from terminal
    #we input the file name (for txt storing beamcount, can be any name)
    #after the space
    #b) when ble_send is launched by this paradigm script
    #the name of the txt file will be composed of the participant name and date
    ble_launch_string = r'pythonw.exe ./BLE_SEND.PY '+default_values['Participant']+'_'+default_values['Date']

    if accuracy == 1 and sched_out == 1:
        if default_values['Bluetooth']: #if default_values about bluetoth is true
            os.system(ble_launch_string) #run the ble_send
        else: #otherwise use the com port connection to send T to te arduino script
            #ser.write(b'ID')  
            SerialHandler.command_to_send(SerialHandler.moveMotor)     

        #corr_sound.play()
        
        for frames in range(int(math.floor(1000/refresh))):
            reward.draw()
            win.flip()
            
        fdbk_dur = fdbk_clock.getTime()
        
        return ('reward',fdbk_onset,fdbk_dur)

    elif accuracy == 1 and sched_out == 0:
        
        #incorr_sound.play()
        for frames in range(int(math.floor(1000/refresh))):
            zero.draw()
            win.flip()

        fdbk_dur = fdbk_clock.getTime()
        return ('zero',fdbk_onset,fdbk_dur)

    elif accuracy == 0 and sched_out == 1:

        #incorr_sound.play()
        for frames in range(int(math.floor(1000/refresh))):
            zero.draw()
            win.flip()

        fdbk_dur = fdbk_clock.getTime()
        return ('zero',fdbk_onset,fdbk_dur)

    elif accuracy == 0 and sched_out == 0:
        if default_values['Bluetooth']:
            os.system(ble_launch_string)
        else:
            #ser.write(b'T')
            SerialHandler.command_to_send(SerialHandler.moveMotor)
        #corr_sound.play()
        for frames in range(int(math.floor(1000/refresh))):
            reward.draw()
            win.flip()

        fdbk_dur = fdbk_clock.getTime()
        if default_values['Bluetooth']:
            pass
        else:
            #ser.write(b'F')   
            print("Placeholder 3")
        return ('reward',fdbk_onset,fdbk_dur)

    elif accuracy == 999:

        for frames in range(int(math.floor(1000/refresh))):
            no_resp.draw()
            win.flip()

        fdbk_dur = fdbk_clock.getTime()
        return ('no_response',fdbk_onset,fdbk_dur)
    
def show_fix(duration,start_time,measured_refresh):
    refresh = measured_refresh
    fix_onset = start_time
    fix_clock = core.Clock()
    fix_clock.reset()
    
    for i in range(duration):
        fix.draw()
        win.flip()

    fix_dur = fix_clock.getTime()

    return (fix_onset,fix_dur)
##Basics for the experiment.
#Window.
wintype='pyglet' 
win = visual.Window([1920,1080], fullscr=default_values['FullScreen'],monitor=TEST_MONITOR, allowGUI = False, color = 'black', winType=wintype) #check window here

#Object, response, fix, and instruction stims.
instruct = visual.TextStim(win, text='Text', alignText = 'center', anchorHoriz = 'center', height = 0.12, wrapWidth = 350, color = 'white')
fix = visual.TextStim(win, text = '+')
left_choice = visual.Circle(win, radius = 0.3, lineColor = 'ForestGreen', lineWidth = 2.0, pos = [-0.4,0])
right_choice = visual.Circle(win, radius = 0.3, lineColor = 'ForestGreen', lineWidth = 2.0, pos = [0.4,0])

#Feedback stims.
reward = visual.ImageStim(win, units = 'norm', size = [1,1], pos = [0,0], image = os.path.join(RESOURCE_PATH,'reward.bmp'))
zero = visual.ImageStim(win, units = 'norm', size = [1,1], pos = [0,0], image = os.path.join(RESOURCE_PATH,'zero.bmp'))
no_resp = visual.TextStim(win, text='No Response Detected!', height = 0.15, wrapWidth = 35, color = 'red')

#Sounds.
#corr_sound = sound.SoundPygame(value=os.path.join(RESOURCE_PATH,'Stimuli/correct.ogg'))
#incorr_sound = sound.SoundPygame(value=os.path.join(RESOURCE_PATH,'Stimuli/incorrect.ogg'))
#advance_sound = sound.SoundPygame(value=os.path.join(RESOURCE_PATH,'Stimuli/click_quiet.ogg'))

#Rating stims.
feedback_image = visual.ImageStim(win, units = 'norm', size = [1,1], pos = [0,0], image = os.path.join(RESOURCE_PATH,'reward.bmp'))
rating_text = visual.TextStim(win, text = 'How Do You Feel Right Now?', pos = [0,0.5], height = 0.18, wrapWidth = 35, color = 'white')
valence_rate = visual.ImageStim(win, units = 'cm', size = [22.44,6.42], pos = [0,-7.0], image = os.path.join(RESOURCE_PATH,'valence2.bmp'))
arousal_rate = visual.ImageStim(win, units = 'cm', size = [22.44,6.42], pos = [0,-7.0], image = os.path.join(RESOURCE_PATH,'arousal2.bmp'))
b0_choice = visual.Rect(win, units = 'cm', width = 3.5, height = 6.2, lineColor = 'ForestGreen', lineWidth = 4.0, pos = [-9.4,-7.0])
b1_choice = visual.Rect(win, units = 'cm', width = 3.5, height = 6.2, lineColor = 'ForestGreen', lineWidth = 4.0, pos = [-4.8,-7.0])
b2_choice = visual.Rect(win, units = 'cm', width = 3.5, height = 6.2, lineColor = 'ForestGreen', lineWidth = 4.0, pos = [-0.2,-7.0])
b3_choice = visual.Rect(win, units = 'cm', width = 3.5, height = 6.2, lineColor = 'ForestGreen', lineWidth = 4.0, pos = [4.4,-7.0])
b4_choice = visual.Rect(win, units = 'cm', width = 3.5, height = 6.2, lineColor = 'ForestGreen', lineWidth = 4.0, pos = [8.9,-7.0])

#Durations (s) for ISI and ITI
#fix_list was generated by random selection from an exponential distribution.
#min = 0.5, max = 8.0, mean = 2.0, mode = 0.5

fix_list = [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0, 4.0, 5.0, 5.0, 5.0, 6.0, 6.0, 6.0, 7.0, 8.0]

#Create isi/iti lists w/durations in screen refreshs, and randomize them.

isi_list = []
iti_list = []

for dur in range(len(fix_list)):
    for list in [isi_list, iti_list]:
        list.append(int(round((fix_list[dur]*1000)/refresh)))

##Set-up the stim/response contingencies.

num_blocks = 4
num_stims = 6
trials_per_stim = 10 # Number times stim on left out of 20 trials.

#Make master list of stim lists.

stim_list = [1 for x in range(num_stims)]
count = 1
for x in range(num_stims):
    stim_list[x] = [count for y in range(trials_per_stim)]
    count+=1

#Assign the individual stim lists to stim names.

A = stim_list[0]
C = stim_list[1]
E = stim_list[2]
F = stim_list[3]
D = stim_list[4]
B = stim_list[5]

#Make the reward probability vectors.

n80 = [1,1,1,1,1,1,1,1,0,0]
n70 = [1,1,1,1,1,1,1,0,0,0]
n60 = [1,1,1,1,1,1,0,0,0,0]

#Concatenate stim lists and reward probability vectors.

AB = np.column_stack([A,B,n80])
BA = np.column_stack([B,A,n80])
CD = np.column_stack([C,D,n70])
DC = np.column_stack([D,C,n70])
EF = np.column_stack([E,F,n60])
FE = np.column_stack([F,E,n60])

AB_trialList = np.vstack([AB,BA])
CD_trialList = np.vstack([CD,DC])
EF_trialList = np.vstack([EF,FE])

#Shuffle trial lists to space out rewards.

np.random.shuffle(AB_trialList)
np.random.shuffle(CD_trialList)
np.random.shuffle(EF_trialList)

#Make 20 "small blocks" with one trial each from the AB, CD, and EF lists.

small_blocks = [[i] for i in range(20)]

for i in range(20):
    small_blocks[i] = np.vstack([AB_trialList[i],CD_trialList[i],EF_trialList[i]])

#Shuffle bitmaps so images used as stims A, B, C, etc. vary across subjects.

pic_list = [os.path.join(RESOURCE_PATH,'1.bmp'), os.path.join(RESOURCE_PATH,'2.bmp'), os.path.join(RESOURCE_PATH,'3.bmp'), os.path.join(RESOURCE_PATH,'4.bmp'), os.path.join(RESOURCE_PATH,'5.bmp'), os.path.join(RESOURCE_PATH,'6.bmp')]
np.random.shuffle(pic_list) 

stim_A = pic_list[0]
stim_C = pic_list[1]
stim_E = pic_list[2]
stim_F = pic_list[3]
stim_D = pic_list[4]
stim_B = pic_list[5]

#Write out stim randomization for use in test.

stim_rand = {'stim_A':pic_list[0], 'stim_C':pic_list[1], 'stim_E':pic_list[2], 'stim_F':pic_list[3], 'stim_D':pic_list[4], 'stim_B':pic_list[5]}

df = pd.DataFrame(stim_rand.items())
df.to_csv(os.path.join(DATA_PATH,'%s_PST_stim_rand.csv'%(default_values['Participant'])), header=False, index=False)
#df.to_csv(default_values['participant']+'_PST_stim_rand.csv', header=False, index=False)

#Clocks.

RT = core.Clock()
fMRI_clock = core.Clock()

#File to collect training data. 

#train_file = default_values['participant'] + '_' + default_values['Date']
train_file = os.path.join(DATA_PATH, '%s_%s_PST_fMRI_train.csv'%(default_values['Participant'], default_values['Date']))
trainFile = open(train_file, 'w')
trainFile.write('block,trial_num,left_stim,left_stim_number,right_stim,right_stim_number,object_onset,'+
                'object_duration,response,response_onset,trial_RT,accuracy,isi_onset,isi_duration,scheduled_outcome,'+
                'feedback,feedback_onset,feedback_duration,iti_onset,iti_duration\n')

##Start the study.
#Instructions.

inst_text = ['This is a new game, with\nchances to win more money.\n\nPress button 1 to advance.', 
'Two figures will appear\non the computer screen.\n\nOne figure will pay you more often\nthan the other, but at first you won\'t\nknow which figure is the good one.\n\nPress 1 to advance.', 'Try to pick the figure that pays\nyou most often.\n\nWhen you see the REWARD screen,\nthat means you won bonus money!\n\nWhen you see the ZERO screen,\nyou did not win.\n\nPress 1 to advance.', 'Keep in mind that no figure\npays you every time you pick it.\n\nJust try to pick the one\nthat pays most often.\n\nPress 1 to select the figure\non the left. Press 4 to select\nthe figure on the right.\n\nPress 1 to advance.', 'At first you may be confused,\nbut don\'t worry.\n\nYou\'ll get plenty of chances.\n\nPress 1 to advance.', 'There are 4 blocks of trials.\nEach one lasts about 8 minutes.\n\nMake sure to try all the figures\nso you can learn which ones\nare better and worse.\n\nPress button 1 to advance.']

allKeys = []

for i in range(len(inst_text)):
    advance = 'false'

    while advance == 'false':
        instruct.setText(text = inst_text[i]) 
        instruct.draw()
        win.flip()
        allKeys = event.waitKeys(keyList = [LEFT_KEY, QUIT_KEY])
        resp = allKeys[0][0]

        if resp == LEFT_KEY:
            #advance_sound.play()
            advance = 'true'
            allKeys = []

        elif resp == QUIT_KEY:
            #advance_sound.play()
            core.quit()

#Run experimental trials.

block_num = 1

for block in range(num_blocks):

    #Shuffle ISI/ITI durations.

    np.random.shuffle(isi_list)
    np.random.shuffle(iti_list)

    #Randomize each small block (scramble AB,CD,EF trios).

    for i in range(20):
        np.random.shuffle(small_blocks[i])

    #Make AllTrials array of small blocks.

    AllTrials = np.asarray(small_blocks)

    #Check no more than 6 consecutive rewards scheduled, otherwise shuffle.

    while not check_rand(AllTrials,20,3):
        np.random.shuffle(AllTrials)

    #Generate lists for leftStims, rightStims, and sch_outcome.
    #sch_outcome different from reward/zero fdbk, which depends on accuracy. 

    leftStims = []
    left_stim_numbers = []
    rightStims = []
    right_stim_numbers = []
    sch_outcome = []

    for x in range(20):
        for y in range(3):
            leftStims.append(AllTrials[x,y,0])
            rightStims.append(AllTrials[x,y,1])
            sch_outcome.append(AllTrials[x,y,2])

    left_stim_numbers = leftStims
    right_stim_numbers = rightStims

    leftStims = [stim_A if x==1 else stim_C if x==2 else stim_E if x==3 else stim_F if x==4 else stim_D if x==5 else stim_B if x==6 else x for x in leftStims]

    rightStims = [stim_A if x==1 else stim_C if x==2 else stim_E if x==3 else stim_F if x==4 else stim_D if x==5 else stim_B if x==6 else x for x in rightStims]

    #Load the stims in a matrix to improve timing/efficiency.
    stim_matrix = {}

    for i in range(len(leftStims)): 
        instruct.setText(text = 'Please relax and hold still\n\nas we set up the computer.\n\nThis will only take a few seconds.')
        instruct.draw()
        win.flip()

        left_stim = visual.ImageStim(win, units = 'norm', size = [0.5,0.5], pos = [-0.4,0], image=leftStims[i])
        left_stim_name = leftStims[i]
        left_stim_number = left_stim_numbers[i]
        right_stim = visual.ImageStim(win, units = 'norm', size = [0.5,0.5], pos = [0.4,0], image=rightStims[i])
        right_stim_name = rightStims[i]
        right_stim_number = right_stim_numbers[i]
        scheduled_outcome = sch_outcome[i] 
        stim_matrix[i] = (left_stim,left_stim_name,left_stim_number,right_stim,right_stim_name,right_stim_number,scheduled_outcome)
    
    #Check-in before starting scan.

    last_text = ['We will check in with you now\n\nto ask if you are ready to begin.']

    advance = 'false'
    k = ['']

    while advance == 'false':
        instruct.setText(text=last_text[0])
        instruct.draw()
        win.flip()
        k = event.waitKeys()

        if k[0] == 'o':
            advance = 'true'

        elif k[0] == 'q':
            core.quit()

    #Disdaqs, start fMRI clock.
    fMRI_clock.reset() #fMRI_clock begins at start of disdaqs.
    while fMRI_clock.getTime() < initial_waittime:
        fix.draw()
        win.flip()

    #Run through the trials.

    for i in range(len(leftStims)):

        trial_num = i + 1

        #Clear buffers.

        event.clearEvents()
        allKeys=[]
        resp=[]
        trial_RT=[]
        stim_frameN = 0

        #Prep the stims.

        left_stim = stim_matrix[trial_num-1][0]
        left_stim_name = stim_matrix[trial_num-1][1]
        left_stim_number = stim_matrix[trial_num-1][2]
        right_stim = stim_matrix[trial_num-1][3]
        right_stim_name = stim_matrix[trial_num-1][4]
        right_stim_number = stim_matrix[trial_num-1][5]
        scheduled_outcome = stim_matrix[trial_num-1][6]

        #Set ISI/ITI durs.

        isi_dur = isi_list[i]
        iti_dur = iti_list[i]

        #Reset the RT clock. 

        RT.reset()

        #Set-up desired trial dur (excluding ITI).

        targ_trial_dur = stim_dur + (isi_dur * refresh)/1000 + fdbk_dur

        #Draw the stims and handle keyboard input.
        
        object_onset = fMRI_clock.getTime()
        while stim_frameN < int(math.floor(3000/refresh)):
            response = 'false'
            left_stim.draw()
            right_stim.draw()
            fix.draw()
            win.flip()
            allKeys=event.getKeys(keyList = [LEFT_KEY,RIGHT_KEY,QUIT_KEY], timeStamped=RT)

            if allKeys:
                resp = allKeys[0][0]
                trial_RT=allKeys[0][1]
                #advance_sound.play()

                if resp == QUIT_KEY:
                    core.quit()

                elif resp == LEFT_KEY:
                    response = 'left'
                    trial_response = show_resp(response,left_stim_number,right_stim_number,stim_frameN,refresh,fMRI_clock.getTime())
                    isi = show_fix(isi_dur,fMRI_clock.getTime(),refresh)
                    object_dur = isi[0] - object_onset
                    feedback = show_fdbk(trial_response[0],scheduled_outcome,response,fMRI_clock.getTime(),refresh)
                    act_trial_dur = object_dur + isi[1] + feedback[2]
                    iti_dur = iti_dur + int(round(((targ_trial_dur - act_trial_dur)*1000)/refresh))
                    iti = show_fix(iti_dur,fMRI_clock.getTime(),refresh)
                    stim_frameN = int(math.floor(3000/refresh))

                elif resp == RIGHT_KEY:
                    response = 'right'
                    trial_response = show_resp(response,left_stim_number,right_stim_number,stim_frameN,refresh,fMRI_clock.getTime())
                    isi = show_fix(isi_dur,fMRI_clock.getTime(),refresh)
                    object_dur = isi[0] - object_onset
                    feedback = show_fdbk(trial_response[0],scheduled_outcome,response,fMRI_clock.getTime(),refresh)
                    act_trial_dur = object_dur + isi[1] + feedback[2]
                    iti_dur = iti_dur + int(round(((targ_trial_dur - act_trial_dur)*1000)/refresh))
                    iti = show_fix(iti_dur,fMRI_clock.getTime(),refresh)
                    stim_frameN = int(math.floor(3000/refresh))
            
            stim_frameN = stim_frameN + 1

        #Catch trials with no response.

        if stim_frameN == int(math.floor(3000/refresh)) and response == 'false':
            response = 'No_response'
            trial_RT = 999
            accuracy = 999
            isi = show_fix(isi_dur,fMRI_clock.getTime(),refresh)
            object_dur = isi[0] - object_onset
            trial_response = (999,999.0)
            feedback = show_fdbk(accuracy,scheduled_outcome,response,fMRI_clock.getTime(),refresh)
            act_trial_dur = object_dur + isi[1] + feedback[2]
            iti_dur = iti_dur + int(round(((targ_trial_dur - act_trial_dur)*1000)/refresh))
            iti = show_fix(iti_dur,fMRI_clock.getTime(),refresh)

        #Write out the data.

        trainFile.write('%i,%i,%s,%i,%s,%i,%0.3f,%0.3f,%s,%0.3f,%0.3f,%i,%0.3f,%0.3f,%i,%s,%0.3f,%0.3f,%0.3f,%0.3f\n' %(block_num, trial_num, left_stim_name, left_stim_number, right_stim_name, right_stim_number, object_onset, object_dur, response, trial_response[1], trial_RT, trial_response[0], isi[0], isi[1], scheduled_outcome, feedback[0], feedback[1], feedback[2], iti[0], iti[1]))

        #Fade out with lastHRF fixation cross after 60 trials.
        
        if trial_num == 60: 
            elapsed_time = fMRI_clock.getTime()
            time_left = end_time - elapsed_time

            for i in range(int(round((time_left*1000)/refresh))):
                fix.draw()
                win.flip()

    #Present a screen between blocks.

    if block_num < num_blocks:

        pause_text = ['Great job!\n\nYou are done with that block.\n\nTake a few seconds to relax.\n\nWhen you are ready to continue,\npress button 1.']
        
        allKeys = []

        for i in range(len(pause_text)):
            advance = 'false'
            instruct.setText(text = pause_text[i]) 
            
            while advance == 'false':
                instruct.draw()
                win.flip()
                allKeys = event.waitKeys(keyList = [LEFT_KEY,QUIT_KEY])
                resp = allKeys[0][0]

                if resp == LEFT_KEY:
                    advance = 'true'
                    #advance_sound.play()
                    allKeys = []

                elif resp == QUIT_KEY:
                    core.quit()

    #Update the block count.

    block_num+=1

#Now that we've looped over all the blocks, close the training file.

trainFile.close()

#Move on to outcome ratings.

rate_text = ['Great job!\n\nNow we\'d like you to rate your emotional\nresponse to the REWARD and ZERO\noutcomes again.\n\nPress 1 to continue.']

allKeys = []

for i in range(len(rate_text)):

    advance = 'false'
    
    while advance == 'false':
        instruct.setText(text=rate_text[i])
        instruct.draw()
        win.flip()
        allKeys = event.waitKeys(keyList=[b1,QUIT_KEY])

        if allKeys:
            resp = allKeys[0][0]

            if resp == b1:
                advance = 'true'
                #advance_sound.play()
                allKeys = []

            else:
                #advance_sound.play()
                core.quit()

#Valence instructions.

valence_rate_inst = ['You will again use all 5 buttons to tell us\nhow PLEASANT or UNPLEASANT you\nfind the reward and zero outcomes.\n\n\nPress 1 to continue.', 'Here\'s how to use the scale:\n\nPress 0 if you feel unhappy,\nunsatisfied, or bored.\n\nPress 2 if you feel neutral,\nnot pleased or displeased.\n\nPress 4 if you feel happy,\nsatisfied, or contented.\n\nUse 1 and 3 for intermediate ratings.\n\nPress 1 to start making your ratings.']

allKeys = []

for i in range(len(valence_rate_inst)):

    advance = 'false'

    while advance == 'false':
        instruct.setText(text=valence_rate_inst[i])
        instruct.draw()
        win.flip()
        allKeys=event.waitKeys(keyList=[b1,QUIT_KEY])

        if allKeys:
            resp = allKeys[0][0]
            
            if resp == b1:
                advance = 'true'
                #advance_sound.play()
                allKeys = []

            else:
                #advance_sound.play()
                core.quit()

#Set-up file to collect ratings data.

ratefile = default_values['subject'] + '_' + default_values['Date']
PST_Rate_Data_File = open(ratefile+'_PST_fMRI_ratings.csv', 'w')
PST_Rate_Data_File.write('stimulus,prompt,rating\n')

#Valence ratings.

prompt = 'valence'

iti_frameN = 0
while iti_frameN <= 120:
    fix.draw()
    win.flip()
    iti_frameN = iti_frameN + 1

outcome_list = ['reward.bmp','zero.bmp']

for i in range(len(outcome_list)):

    allKeys = []
    fdbk_frameN = 0
    rate_frameN = 0
    advance = 'false'

    feedback_image.setImage(value=os.path.join(RESOURCE_PATH, outcome_list[i]))

    if outcome_list[i] == 'reward.bmp':
        outcome = 'reward'
        #corr_sound.play()
        while fdbk_frameN <= 120:
            feedback_image.draw()
            win.flip()
            fdbk_frameN = fdbk_frameN + 1

        rating_text.draw()
        valence_rate.draw()
        win.flip()

    else:
        outcome = 'zero'
        #incorr_sound.play()
        while fdbk_frameN <= 120:
            feedback_image.draw()
            win.flip()
            fdbk_frameN = fdbk_frameN + 1

        rating_text.draw()
        valence_rate.draw()
        win.flip()

    while advance == 'false':
        allKeys=event.waitKeys(keyList=[b0,b1,b2,b3,b4])
        rating_text.draw()
        valence_rate.draw()
        win.flip()
        
        if allKeys:
            resp = allKeys[0][0]
            if resp == b0:
                advance = 'true'
                #advance_sound.play()
                rating = 0
                while rate_frameN <= 60:
                    rating_text.draw()
                    valence_rate.draw()
                    b0_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b1:
                advance = 'true'
                #advance_sound.play()
                rating = 1
                while rate_frameN <= 60:
                    rating_text.draw()
                    valence_rate.draw()
                    b1_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b2:
                advance = 'true'
                #advance_sound.play()
                rating = 2
                while rate_frameN <= 60:
                    rating_text.draw()
                    valence_rate.draw()
                    b2_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b3:
                advance = 'true'
                #advance_sound.play()
                rating = 3
                while rate_frameN <= 60:
                    rating_text.draw()
                    valence_rate.draw()
                    b3_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b4:
                advance = 'true'
                #advance_sound.play()
                rating = 4
                while rate_frameN <= 60:
                    rating_text.draw()
                    valence_rate.draw()
                    b4_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

        #Record the valence ratings.

        PST_Rate_Data_File.write('%s,%s,%i\n' %(outcome, prompt, rating))
        
        #Present fixation.

        iti_frameN = 0
        while iti_frameN <= 60:
            fix.draw()
            win.flip()
            iti_frameN = iti_frameN + 1

#Arousal instructions.

arousal_rate_inst = ['Now you will (again) use a different 5-point\nscale to tell us how AROUSING you find\nthe reward and zero outcomes.\n\nPress 1 to continue.', 'Here\'s how to use the 5-point scale:\n\nPress 0 if you feel relaxed,\nsluggish, or sleepy.\n\nPress 2 if you feel moderate arousal:\nnot very calm, but not very excited.\n\nPress 4 if you feel excited,\njittery, or wide awake.\n\nUse 1 and 3 for intermediate ratings.\n\nPress 1 to start making your ratings.']

#Arousal ratings.

allKeys = []

for i in range(len(arousal_rate_inst)):

    advance = 'false'

    while advance == 'false':
        instruct.setText(text=arousal_rate_inst[i])
        instruct.draw()
        win.flip()
        allKeys=event.waitKeys(keyList=[b1,QUIT_KEY])

        if allKeys:
            resp = allKeys[0][0]
            
            if resp == b1:
                #advance_sound.play()
                advance = 'true'

            else:
                core.quit()

#Arousal ratings.

prompt = 'arousal'

iti_frameN = 0
while iti_frameN <= 120:
    fix.draw()
    win.flip()
    iti_frameN = iti_frameN + 1

outcome_list = ['reward.bmp','zero.bmp']

for i in range(len(outcome_list)):

    allKeys = []
    fdbk_frameN = 0
    rate_frameN = 0
    advance = 'false'

    feedback_image.setImage(value=outcome_list[i])

    if outcome_list[i] == 'reward.bmp':
        outcome = 'reward'
        #corr_sound.play()
        while fdbk_frameN <= 120:
            feedback_image.draw()
            win.flip()
            fdbk_frameN = fdbk_frameN + 1

        rating_text.draw()
        arousal_rate.draw()
        win.flip()

    else:
        outcome = 'zero'
        #incorr_sound.play()
        while fdbk_frameN <= 120:
            feedback_image.draw()
            win.flip()
            fdbk_frameN = fdbk_frameN + 1

        rating_text.draw()
        arousal_rate.draw()
        win.flip()

    while advance == 'false':
        allKeys=event.waitKeys(keyList=[b0,b1,b2,b3,b4])
        rating_text.draw()
        arousal_rate.draw()
        win.flip()
        
        if allKeys:
            resp = allKeys[0][0]
            if resp == b0:
                advance = 'true'
                #advance_sound.play()
                rating = 0
                while rate_frameN <= 60:
                    rating_text.draw()
                    arousal_rate.draw()
                    b0_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b1:
                advance = 'true'
                #advance_sound.play()
                rating = 1
                while rate_frameN <= 60:
                    rating_text.draw()
                    arousal_rate.draw()
                    b1_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b2:
                advance = 'true'
                #advance_sound.play()
                rating = 2
                while rate_frameN <= 60:
                    rating_text.draw()
                    arousal_rate.draw()
                    b2_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b3:
                advance = 'true'
                #advance_sound.play()
                rating = 3
                while rate_frameN <= 60:
                    rating_text.draw()
                    arousal_rate.draw()
                    b3_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

            if resp == b4:
                advance = 'true'
                #advance_sound.play()
                rating = 4
                while rate_frameN <= 60:
                    rating_text.draw()
                    arousal_rate.draw()
                    b4_choice.draw()
                    win.flip()
                    rate_frameN = rate_frameN + 1

        #Record the arousal ratings.

        PST_Rate_Data_File.write('%s,%s,%i\n' %(outcome, prompt, rating))

        #Present fixation.

        iti_frameN = 0
        while iti_frameN <= 60:
            fix.draw()
            win.flip()
            iti_frameN = iti_frameN + 1

#Close the rating file.

PST_Rate_Data_File.close()
