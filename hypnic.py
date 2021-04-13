# coding=utf-8

# sample script for the Raspberry Pi
# this script needs to be loaded on boot
 
import RPi.GPIO as GPIO
import os
import time
 
def power_callback(channel):
    if GPIO.input(channel) == GPIO.LOW:
        print('Powering off...')
        os.system('sudo shutdown -h now')
 
try:
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.add_event_detect(17, GPIO.BOTH, callback=power_callback)
 
    while True:
        time.sleep(5)
 
finally:
    GPIO.cleanup()