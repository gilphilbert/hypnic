/* CLOCKWISE
#define PA0     0 // physical pin 13
#define PA1     1 // physical pin 12
#define PA2     2 // physical pin 11
#define PA3     3 // physical pin 10
#define PA4     4 // physical pin 9
#define PA5     5 // physical pin 8
#define PA6     6 // physical pin 7
#define PA7     7 // physical pin 6
#define PB0     10 // physical pin 2
#define PB1     9 // physical pin 3
#define PB2     8 // physical pin 5
#define PB3     11 // physical pin 4
*/
#define PA0     10 // physical pin 13
#define PA1     9 // physical pin 12
#define PA2     8 // physical pin 11
#define PA3     7 // physical pin 10
#define PA4     6 // physical pin 9
#define PA5     5 // physical pin 8
#define PA6     4 // physical pin 7
#define PA7     3 // physical pin 6
#define PB0     0 // physical pin 2
#define PB1     1 // physical pin 3
#define PB2     2 // physical pin 5
#define PB3     11 // physical pin 4

#define A0      PA0
#define A1      PA1
#define A2      PA2
#define A3      PA3
#define A4      PA4
#define A5      PA5
#define A6      PA6
#define A7      PA7

#define D0      PA0
#define D1      PA1
#define D2      PA2
#define D3      PA3
#define D4      PA4
#define D5      PA5
#define D6      PA6
#define D7      PA7
#define D8      PB2
#define D9      PB1
#define D10     PB0
#define D11     PB3

#define MISO    PA5
#define MOSI    PA6
#define SCK     PA4
#define RESET   PB3

#define DI      PA6
#define DO      PA5

#define SDA     PA6
#define SCL     PA4

#define PCINT0  PA0
#define PCINT1  PA1
#define PCINT2  PA2
#define PCINT3  PA3
#define PCINT4  PA4
#define PCINT5  PA5
#define PCINT6  PA6
#define PCINT7  PA7

#define AREF    PA0

/*

      ATTINY24/44/84
         --------
VCC  1 -| o      |- 14 GND
PB0  2 -|        |- 13 PA0
PB1  3 -|        |- 12 PA1
PB3  4 -|        |- 11 PA2
PB2  5 -|        |- 10 PA3
PA7  6 -|        |- 9  PA4
PB6  7 -|        |- 8  PA5
         --------
*/