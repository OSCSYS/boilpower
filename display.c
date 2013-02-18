#include "display.h"
#include <avr/interrupt.h>
#include <util/atomic.h> 

//Character definitions
//PORT BIT TO SEGMENT MAP: ED.C GBFA

static const uint8_t kCharDigitMap[] = { 0xd7, //0
                                  0x14, //1
                                  0xcd, //2
                                  0x5d, //3
                                  0x1e, //4
                                  0x5b, //5
                                  0xdb, //6
                                  0x15, //7
                                  0xdf, //8
                                  0x5f  //9
};

static const uint8_t kCharDecimalPoint = 0x20;
static const uint8_t kCharA  =           0x9f;
static const uint8_t kCharb  =           0xda;
static const uint8_t kCharC  =           0xc3;
static const uint8_t kCharc  =           0xc8;
static const uint8_t kChard  =           0xdc;
static const uint8_t kCharE  =           0xcb;
static const uint8_t kCharF  =           0x8b;
static const uint8_t kCharg  =           0x5f;
static const uint8_t kCharH  =           0x9e;
static const uint8_t kCharh  =           0x9a;
static const uint8_t kChari  =           0x80;
static const uint8_t kCharJ  =           0xd4;
static const uint8_t kCharL  =           0xc2;
static const uint8_t kCharN  =           0x97;
static const uint8_t kCharn  =           0x98;
static const uint8_t kCharo  =           0xd8;
static const uint8_t kCharP  =           0x8f;
static const uint8_t kCharr  =           0x88;
static const uint8_t kCharU  =           0xd6;
static const uint8_t kCharu  =           0xd0;

//Digit select port bit mapping
static const uint8_t kDigitSelect[3] = {_BV(5), _BV(4), _BV(3)};

//Port funtions
#define PORT_DIGIT_SELECT PORTC
#define PORT_CHAR         PORTD

//Port direction registers
#define DDR_DIGIT_SELECT DDRC
#define DDR_CHAR         DDRD

//Port bitmasks
static const uint8_t kPinsDigitSelect  = 0x38;
static const uint8_t kPinsChar         = 0xff;

//Global digit values for timer interrupt ISRs
static uint8_t gDigitValue[3] = {0, 0, 0};

//Global Digit Scan Cursor Position for ISR: 0-2
static volatile uint8_t gDigitCursor = 0;

//Global millis counter
static volatile uint32_t gMillis = 0;

void display_init(void) {
  //Set pin directions
  DDR_DIGIT_SELECT |= kPinsDigitSelect;
  DDR_CHAR |= kPinsChar;

  // Configure timer 0 for CTC mode 
  TCCR0A |= _BV(WGM01);

  // Enable CTC Timer0 Compare A interrupt
  TIMSK0 |= _BV(OCIE0A);

  //  Enable global interrupts 
  sei();
  
  // Set compare value to 125 for a compare rate of 1kHz 
  OCR0A = 125;
  
  //Set Timer0 Prescaler to 64
  TCCR0B |= (_BV(CS00) | _BV(CS01));
}

void display_on(void) {
}

void display_off(void) {
}

void display_write_number(int number) {
  if (number > 999 || number < 0) return;
  int remainder = number;
  uint8_t divisor = 100;
  for(int position = 2; position > -1; --position) {
    if (number >= divisor) {
      uint8_t value = remainder / divisor;
      gDigitValue[position] = kCharDigitMap[value];
      remainder -= value * divisor;
    } else {
      gDigitValue[position] = 0;
    }
    divisor /= 10;
  }
}

void display_write_decimalpoint(uint8_t precision) {
  for(uint8_t position = 0; position < 3; ++position) {
    if (precision == position + 1) {
      gDigitValue[position] |= kCharDecimalPoint;
    } else {
      gDigitValue[position] &= ~kCharDecimalPoint;
    }
  }
}

uint32_t display_millis(void) {
  unsigned long ms;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    ms = gMillis;
  } 
  return ms;
}

ISR(TIMER0_COMPA_vect) 
{
  //Increment global millis counter
  ++gMillis;

  //Bring all digit select pins high
  PORT_DIGIT_SELECT |= kPinsDigitSelect;
  //Write char value
  PORT_CHAR = gDigitValue[gDigitCursor];
  //Bring current digit select pin low
  PORT_DIGIT_SELECT &= ~(kDigitSelect[gDigitCursor++]);
  if (gDigitCursor > 2) { gDigitCursor = 0; }
}