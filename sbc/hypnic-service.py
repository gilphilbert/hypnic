#               DO NOT EDIT THIS FILE              #
# -------------------------------------------------#
# If you wish to modify the pins used, please edit #
#    /etc/hypnic.conf                              #
#--------------------------------------------------#

from ConfigParser import SafeConfigParser

class GlobalSection(object):
    def __init__(self, fp):
        self.fp = fp
        self.globalSec = '[pins]\n'

    def readline(self):
        if self.globalSec:
            try:
                return self.globalSec
            finally:
                self.globalSec = None
        else:
            return self.fp.readline()

parser = SafeConfigParser()
parser.readfp(GlobalSection(open('/etc/hypnic.conf')))

HALT_PIN = parser.getint("pins", "halt_pin")

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
    GPIO.setup(HALT_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    # if the pin falls, call the powerdown command
    GPIO.add_event_detect(HALT_PIN, GPIO.FALLING, callback=powerOff)

    # sleep for 5ms each loop to prevent wasting CPU time
    while True:
        time.sleep(5)

finally:
    # clean GPIOs when we die
    GPIO.cleanup()