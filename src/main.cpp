#include <Arduino.h>

#define HALT_MESSAGE    8 // physical pin 5
#define POWER_CONTROL   9 // physical pin 6
#define HALTED          7 // physical pin 7
#define POWER_BUTTON    6 // physical pin 2
#define EXTERNAL_POWER  0 // physical pin 3
#define CHARGE_LEVEL    3

#define BROWNOUT_DELAY  2000 // wait 2 seconds
#define POWER_DELAY     10000 // wait 10 seconds

bool externalPower = LOW; // assume the device is powered down
bool buttonState = HIGH; // internal pullup, so high is open
unsigned long deviceOnTime = 0; // store the time that the device was powered on
unsigned long brownoutTimer = 0; // timer used to delay power down to cover brownouts
unsigned long powerDownTimer = 0; // powers down the device even if we don't get a safe signal

void setup() {
  // configure the device pins
  pinMode(HALT_MESSAGE, OUTPUT);
  pinMode(POWER_CONTROL, OUTPUT);
  pinMode(HALTED, INPUT_PULLUP);
  pinMode(POWER_BUTTON, INPUT_PULLUP);
  pinMode(EXTERNAL_POWER, INPUT);
  pinMode(CHARGE_LEVEL, INPUT);

  // make sure power is off
  digitalWrite(POWER_CONTROL, 0);
  digitalWrite(HALT_MESSAGE, 0);
}

void checkPower() {
  bool newExternalPower = digitalRead(EXTERNAL_POWER);
  if (newExternalPower != externalPower) {
    externalPower = newExternalPower;
    if (newExternalPower == LOW) {
      // if the power has just gone, start the power down sequence
      brownoutTimer = millis() + BROWNOUT_DELAY;
    } else {
      if (brownoutTimer) {
        // if we're in a suspected brownout (power restored before halt message sent)
        // just stop the timer
        brownoutTimer = 0;
      } else if (powerDownTimer == 0) {
        // if the powerDownTimer is 0 (i.e., we've got power and we're not powering down)
        // if we're not in the middle of powering down, power the device up
        digitalWrite(POWER_CONTROL, 1);
        deviceOnTime = millis();
      }
    }
  }
}

unsigned long buttonDebounce;
bool lastButtonState = HIGH;
void checkButton() {
  bool newState = digitalRead(POWER_BUTTON);
  if (newState != lastButtonState && millis() > buttonDebounce && powerDownTimer == 0) {
    buttonDebounce = millis() + 250; // long debounce, but that's OK for our application
    if (newState == LOW) {
      // button was pressed! Let's see what state we're in
      bool powerState = digitalRead(POWER_CONTROL);
      if (powerState == LOW && externalPower == HIGH) {
        digitalWrite(POWER_CONTROL, 1);
        deviceOnTime = millis();
      } else if (powerDownTimer == 0) {
        digitalWrite(HALT_MESSAGE, 1);
        brownoutTimer = 0;
        powerDownTimer = millis() + POWER_DELAY;
      }
    }
  }
}

void loop() {
  checkPower();
  checkButton();
  if (deviceOnTime && brownoutTimer > 0 && millis() > brownoutTimer) {
    if (externalPower == LOW) {
      // the power is still low, reset the timer and send HALT signal
      brownoutTimer = 0;
      // start the powerDown timer (in case we don't get a message from the device)
      if (powerDownTimer == 0) {
        digitalWrite(HALT_MESSAGE, 1);
        powerDownTimer = millis() + POWER_DELAY;
      }
    }
  }

  bool haltedState = digitalRead(HALTED);
  // if the device is halted, or the powerDownTimer has expired
  if (millis() > deviceOnTime + 5000 && haltedState == LOW || (powerDownTimer && millis() > powerDownTimer)) {
    // reset the system
    powerDownTimer = 0;
    brownoutTimer = 0;
    deviceOnTime = 0;
    digitalWrite(POWER_CONTROL, 0);
    digitalWrite(HALT_MESSAGE, 0);
  }
}
