# coding=utf-8

# ------------------------------------------------------
# Use this variable to define the GPIO pin you have
# connected the "shutdown" pin on the Hypnic board to
# this computer. Make sure to use BCM pins!
# ------------------------------------------------------
STATEPIN = 17


# ------------------------------------------------------
# DO NOT MODIFY THE CODE BELOW THIS LINE
# ------------------------------------------------------

import RPi.GPIO as GPIO
import os
import time

def powerOff(channel):
    print('Powering off...')
    os.system('sudo shutdown -h now')
 
try:
    # for RPi, use BCM mode
    GPIO.setmode(GPIO.BCM)
    # configure the pin to use pullup mode (hypnic will pull this pin down to ground)
    GPIO.setup(STATEPIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    # if the pin falls, call the powerdown command
    GPIO.add_event_detect(STATEPIN, GPIO.FALLING, callback=powerOff)
 
    # sleep for 5ms each loop to prevent wasting CPU time
    while True:
        time.sleep(5)
 
finally:
    # clean GPIOs when we die
    GPIO.cleanup()
