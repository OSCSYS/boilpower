#include "display.h"

#include <avr/interrupt.h>
#include <util/atomic.h> 

#include "hwprofile.h"

//Character definitions (PORT BIT TO SEGMENT MAP: ED.C GBFA)
static const uint8_t kCharTable[] = { 0xd7, //0
                                      0x14, //1
                                      0xcd, //2
                                      0x5d, //3
                                      0x1e, //4
                                      0x5b, //5
                                      0xdb, //6
                                      0x15, //7
                                      0xdf, //8
                                      0x5f, //9
                                      0x9f, //A
                                      0xda, //b
                                      0xc3, //C (alt. c 0xc8)
                                      0xdc, //d
                                      0xcb, //E
                                      0x8b, //F
                                      0xd3, //G
                                      0x9e, //H (alt. h 0x9a)
                                      0x82, //i
                                      0xd4, //J
                                      0x00, //K NOT SUPPORTED
                                      0xc2, //L
                                      0x00, //M NOT SUPPORTED
                                      0x97, //N (alt. n 0x98)
                                      0xd8, //o
                                      0x8f, //P
                                      0x00, //Q NOT SUPPORTED
                                      0x88, //r
                                      0x5b, //S (dupe of 5)
                                      0xca, //t
                                      0xd6  //U (alt. u 0xd0)
};

static const uint8_t kCharDecimal = 0x20;

//Global Char values for timer interrupt ISRs
static uint8_t gDisplayCharBuffer[DISPLAY_CHAR_COUNT];

//Global Char Scan Cursor Position for ISR: 0-2
static volatile uint8_t gDisplayCharCursor = 0;

//Global millis counter
static volatile uint32_t gDisplayMillis = 0;

void display_init(void) {
  DISPLAY_CHAR_SELECT_DIR_REG |= kDisplayCharSelectPinMask;       //Enable Digit Select Pins as outputs
  DISPLAY_CHAR_DIR_REG |= kDisplayCharPinMask;                    //Enable Char pins as outputs
  DISPLAY_TIMER_CONFIG_A_REG |= kDisplayTimerMode;                //Configure timer for CTC mode 
  DISPLAY_TIMER_INTERRUPT_MASK_REG |= kDisplayTimerInterruptMask; //Enable timer interrupt
  sei();                                                          //Enable global interrupts 
  DISPLAY_TIMER_COMPARE_VALUE_REG = kDisplayTimerCompareValue;    //Set compare value for a compare rate of 1kHz 
  DISPLAY_TIMER_CONFIG_B_REG |= kDisplayTimerPrescaler;           //Set timer prescaler
}

void display_on(void) {
}

void display_off(void) {
}

void display_write_number(int number) {
  if (number > 999 || number < 0) return;
  int remainder = number;
  uint8_t divisor = 100;
  for(uint8_t position = DISPLAY_CHAR_COUNT; position; --position) {
    if (number >= divisor) {
      uint8_t value = remainder / divisor;
      gDisplayCharBuffer[position - 1] = kCharTable[value];
      remainder -= value * divisor;
    } else {
      gDisplayCharBuffer[position - 1] = 0;
    }
    divisor /= 10;
  }
  if (number == 0) {
    gDisplayCharBuffer[0] = kCharTable[0];
  }
}

void display_write_string(char *text) {
  uint8_t cursor = DISPLAY_CHAR_COUNT;
  while(*text != '\0' && cursor) {
    uint8_t bmp = 0x00;
    if(*text > 47 && *text < 58) {
      bmp = kCharTable[*text - 48];        //Handle Digits
    } else if (*text > 64 && *text < 86) {
      bmp = kCharTable[*text - 55];        //Handle A-U
    } else if (*text > 96 && *text < 118) {
      bmp = kCharTable[*text - 87];        //Handle A-U
    }
    gDisplayCharBuffer[--cursor] = bmp;
    ++text;
  }
}

void display_write_decimalpoint(uint8_t position) {
  for(uint8_t cursor = DISPLAY_CHAR_COUNT; cursor; --cursor) {
    if (position == cursor) {
      gDisplayCharBuffer[cursor - 1] |= kCharDecimal;
    } else {
      gDisplayCharBuffer[cursor - 1] &= kCharDecimal;
    }
  }
}

uint32_t millis(void) {
  unsigned long ms;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    ms = gDisplayMillis;
  } 
  return ms;
}

ISR(TIMER0_COMPA_vect) 
{
  //Increment global millis counter
  ++gDisplayMillis;

  //Bring all digit select pins high
  DISPLAY_CHAR_SELECT_OUTPUT_REG |= kDisplayCharSelectPinMask;
  //Write char value
  DISPLAY_CHAR_OUTPUT_REG = gDisplayCharBuffer[gDisplayCharCursor];
  //Bring current digit select pin low
  DISPLAY_CHAR_SELECT_OUTPUT_REG &= ~(kDisplayCharSelect[gDisplayCharCursor++]);
  if (gDisplayCharCursor == DISPLAY_CHAR_COUNT) { gDisplayCharCursor = 0; }
}