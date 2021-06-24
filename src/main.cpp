#include <Arduino.h>
// we include our own pin mapping since the Arduino one is poor
#include "pins.h"

#define HALT_MESSAGE    PA2
#define POWER_CONTROL   PA1
#define SAFE            PA3
#define POWER_BUTTON    PA4
#define EXTERNAL_POWER  PB0
#define CHARGE_LEVEL    PA7
#define LED             PB2
#define JUMPERS         PA0

#define BROWNOUT_DELAY  2000 // wait 2 seconds
#define POWER_DELAY     10000 // wait 10 seconds

bool externalPower = LOW; // assume the device is powered down
bool buttonState = HIGH; // internal pullup, so high is open
bool powerState = false; // the current state of the device
bool deviceReportsOn = false; // this is set to 1 when the "SAFE" line goes low
unsigned long brownoutTimer = 0; // timer used to delay power down to cover brownouts
unsigned long powerDownTimer = 0; // powers down the device even if we don't get a safe signal

void setup() {
  // configure the device pins
  pinMode(HALT_MESSAGE, OUTPUT);
  pinMode(POWER_CONTROL, OUTPUT);
  pinMode(SAFE, INPUT_PULLUP);
  pinMode(POWER_BUTTON, INPUT_PULLUP);
  pinMode(EXTERNAL_POWER, INPUT);
  pinMode(CHARGE_LEVEL, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(JUMPERS, INPUT_PULLUP);

  // make sure power is off
  digitalWrite(POWER_CONTROL, 0);
  digitalWrite(LED, 0);
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
        digitalWrite(LED, 1);
        powerState = true;
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
      if (!powerState && externalPower == HIGH) {
        digitalWrite(POWER_CONTROL, 1);
        digitalWrite(LED, 1);
        powerState = true;
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
  if (powerState && brownoutTimer > 0 && millis() > brownoutTimer) {
    if (externalPower == LOW) {
      // the power is still low, reset the timer and send HALT signal
      brownoutTimer = 0;
      // start the powerDown timer (in case we don't get a message from the device)
      if (powerDownTimer == 0) {
        digitalWrite(HALT_MESSAGE, 1);
        // subtract the brownout delay (already waited this long)
        powerDownTimer = millis() + (POWER_DELAY - BROWNOUT_DELAY);
      }
    }
  }

  // since there's no pullup on the halt pin, we need to wait for the SBC to pull it high (a low for us)
  // before we start to monitor it. In the SBC script, the pin must be pulled high.
  bool haltedState = digitalRead(SAFE);
  if (!deviceReportsOn && haltedState == HIGH) {
    deviceReportsOn = true;
  }

  // if the device is halted, or the powerDownTimer has expired
  if ((deviceReportsOn && haltedState == LOW) || (powerDownTimer > 0 && millis() > powerDownTimer)) {
    // reset the system
    powerDownTimer = 0;
    brownoutTimer = 0;
    digitalWrite(POWER_CONTROL, 0);
    digitalWrite(LED, 0);
    powerState = false;
    deviceReportsOn = false;
    digitalWrite(HALT_MESSAGE, 0);
  }
}
