# coding=utf-8
 
import RPi.GPIO as GPIO
import os
import time
 
STATEPIN = 17

def power_callback(channel):
    print('Powering off...')
    os.system('sudo shutdown -h now')
 
try:
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(STATEPIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.add_event_detect(STATEPIN, GPIO.FALLING, callback=power_callback)
 
    while True:
        time.sleep(5)
 
finally:
    GPIO.cleanup()
