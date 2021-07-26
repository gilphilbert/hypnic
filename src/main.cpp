#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//----------- USER CONFIGURABLE -----------//
// When power is applied to the Hypnic, the device is powered on. Comment to disable
//#define AUTO_POWER_ON

// Time in seconds after external power lost the device remains powered up for if no SAFE signal recieved
// Assuming there is enough charge in the ultracapacitors to support the SBC for this long
#define POWER_LOST_TIMEOUT  20

//----- SYSTEM STATIC - DO NOT CHANGE -----//
#define POWER_CONTROL       PA1
#define HALT_MESSAGE        PA2
#define SAFE                PA3
#define POWER_BUTTON        PA4
#define CHARGE_LEVEL        PA7

#define EXTERNAL_POWER      PB0
#define LED                 PB2

// states the hypnic can be in
#define STATE_IDLE      0           // usual state
#define STATE_BROWNOUT  1           // external power has been lost for <2s
#define STATE_POWERDOWN 2           // graceful shutdown started but not complete

uint8_t currentState = STATE_IDLE;  // shows the current "state" (action we're performing)
uint8_t powerDownTimer = 0;         // used to count the number of ticks while we're waiting for power down
bool isDebounce = 0;                // is the button being debounced?

// -------------------------------------- //
//  configures the IO pins                //
// -------------------------------------- //
void pinConfig() {
  DDRA |= (1 << POWER_CONTROL);   // device control as output
  DDRA |= (1 << HALT_MESSAGE);    // halt as output
  DDRA &= ~(1 << SAFE);           // safe as input
  DDRA &= ~(1 << POWER_BUTTON);   // power_button as input
  DDRA &= ~(1 << CHARGE_LEVEL);   // charge_level as input

  PORTA |= (1 << SAFE);           // pull up required
  PORTA |= (1 << POWER_BUTTON);   // pull up required

  DDRB &= ~(1 << EXTERNAL_POWER); // external power as input
  DDRB |= (1 << LED);             // LED as output
}

// -------------------------------------- //
//  configures the interrupts             //
// -------------------------------------- //
void interruptConfig() {
  GIMSK  |= (1 << PCIE0);           // enable PCINT 0-7
  PCMSK0 |= (1 << POWER_BUTTON);    // interrupt for external power
  PCMSK0 |= (1 << SAFE);            // interrupt for safe message
  GIMSK  |= (1 << PCIE1);           // enable PCINT 8-11
  PCMSK1 |= (1 << EXTERNAL_POWER);  // interrupt for external power
  sei();                            // enable interrupts
}

// -------------------------------------- //
//  configures the timers                 //
// -------------------------------------- //
void timerConfig() {
  // timer1 is used for brownout delay and shutdown delay (with 2Hz ticks)
  TCCR1B |= (1 << WGM12);                               // Configure timer 1 for CTC mode
  TIMSK1 |= (1 << OCIE1A);                              // Enable CTC interrupt
  TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));  // prescaler of 0 disables the timer

  // timer0 is used for button debounce
  TCCR0B |= (1 << WGM12);                                // Configure timer 0 for CTC mode
  TIMSK0 |= (1 << OCIE0A);                               // Enable CTC interrupt
  OCR0A   = 195;                                         // at 1Mhz and prescaler of 8, this will give us 0.5s tick
  TCCR0B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));  // prescaler of 0 disables the timer
}

// ---------------------------------------- //
//      changes the state of the device     //
// ---------------------------------------- //
//  when turning off, also resets the state //
//  and disables the timer                  //
// ---------------------------------------- //
void changePowerState(bool state) {
  if (state == 0) {
    // turn off pins
    PORTA &= ~(1 << POWER_CONTROL);
    PORTB &= ~(1 << LED);
    PORTA &= ~(1 << HALT_MESSAGE);  // clear the halt message to the device
    currentState = STATE_IDLE;
    // set the counter back to zero
    powerDownTimer = 0;
    TCCR1B &= ~((1 << CS10) | (1 << CS11) |(1 << CS12)); // prescaler of 0
  } else {
    // turn on pins
    PORTA |= (1 << POWER_CONTROL);
    PORTB |= (1 << LED);
  }
}

// -------------------------------------- //
//  starts the graceful shutdown process  //
// -------------------------------------- //
void startPowerdown() {
  PORTA |= (1 << HALT_MESSAGE);     // send the halt message to the device
  currentState = STATE_POWERDOWN;   // change our current state
  // now set the timer. it will count to 20(s) flashing the LED at 2Hz
  OCR1A   = 62500;                  // at 1Mhz and prescaler of 8, this will give us 0.5s tick
  TCCR1B |= (1 << CS11);            // prescaler of 8
}

ISR(TIM1_COMPA_vect) {
  switch (currentState) {
    case STATE_BROWNOUT:
      // disable the timer
      TCCR1B &= ~((1 << CS10) | (1 << CS11) |(1 << CS12)); // prescaler of 0

      // is the power still out?
      if (!(PINB & _BV(EXTERNAL_POWER)) ? 1 : 0) {
        startPowerdown();
      }
      // otherwise, power is back and we do nothing
      break;

    case STATE_POWERDOWN:
      // increment the timer
      powerDownTimer++;
      // see if 20s has passed
      if (powerDownTimer < ((POWER_LOST_TIMEOUT * 2) - 4)) {
        // toggle the LED
        PORTB ^= (1 << LED);
      } else {
        // now hard power down the device, it's had 20s! (this will also stop the timer and reset state)
        changePowerState(0);
      }
      break;

    case STATE_IDLE: // used when power is restored during a brownout
      TCCR1B &= ~((1 << CS10) | (1 << CS11) |(1 << CS12)); // prescaler of 0
      break;
  }
}

// handles button debounce
ISR(TIM0_COMPA_vect) {
  TCCR0B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));  // prescaler of 0 disables the timer
  isDebounce = 0;
  bool btnState = (PINA & _BV(POWER_BUTTON)) ? 1 : 0;
  if (btnState == 0) {
    // get the current state of the device
    bool deviceState = (PINA & _BV(POWER_CONTROL)) ? 1 : 0;
    // get the state of the external power
    bool powerState = (PINB & _BV(EXTERNAL_POWER)) ? 1 : 0;
    // if power state is high and the device is off
    if (!deviceState && powerState) {
      // check we're not in brownout / shutdown
      if (currentState == STATE_IDLE) {
        // clear the brownout timer
        changePowerState(1);
      }
    // is the device on?
    } else if (deviceState) {
      // turn it off
      startPowerdown();
    }
  }
}

// handles power button presses and safe messages
ISR (PCINT0_vect) {
  // read safe pin, check to see if it's been pressed
  bool safeState = (PINA & _BV(SAFE)) ? 1 : 0;
  if (safeState == 0) {
    // power off the device
    changePowerState(0);
  }

  // read switch, check to see if it's been pressed
  bool btnState = (PINA & _BV(POWER_BUTTON)) ? 1 : 0;
  if (btnState == 0) {
    if (!isDebounce) {
      TCNT0 = 0;
      isDebounce = 1;
      TCCR0B |= (1 << CS12);  // prescaler of 256 results in 50ms timer
    } else {
      TCNT0 = 0;
    }
  }
}


// handles external power state
ISR (PCINT1_vect) {
  // get the state of the external power
  bool powerState = (PINB & _BV(EXTERNAL_POWER)) ? 1 : 0;
  // if power state has gone high
  if (powerState) {
    // check we're not in the middle of a powerdown right now
    if (currentState != STATE_POWERDOWN) {
      // clear the brownout timer
      currentState = STATE_IDLE;
      #ifdef AUTO_POWER_ON
      changePowerState(1);
      #endif
    }
  // power state has gone low
  } else {
    bool deviceState = (PINA & _BV(POWER_CONTROL)) ? 1 : 0;
    // if the device is currently powered on
    if (deviceState) {
      // start the brownout timer
      currentState = STATE_BROWNOUT;
      OCR1A   = 31250; // at 1Mhz and prescaler of 64, this will give us 2s tick
      TCCR1B |= ((1 << CS10) | (1 << CS11)); // prescaler of 64
    }
  }
}

int main() {
  pinConfig();
  interruptConfig();
  timerConfig();

  PORTA &= ~(1 << HALT_MESSAGE); // clear any request to halt
  #ifdef AUTO_POWER_ON
  // if configured for autopoweron, do it now
  PORTA |= (1 << POWER_CONTROL); // power is turned on
  PORTB |= (1 << LED); // led is turned on
  #else
  // otherwise we make sure pins are low
  PORTA &= ~(1 << POWER_CONTROL); // power is turned off
  PORTB &= ~(1 << LED); // led is turned off
  #endif

  while(1) {


  }
}