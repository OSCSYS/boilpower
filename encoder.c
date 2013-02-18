#include "encoder.h"
#include <avr/interrupt.h>
#include <util/atomic.h> 

//Encoder pin mapping
static const uint8_t kEncoderA = _BV(0);
static const uint8_t kEncoderB = _BV(1);
static const uint8_t kEncoderE = _BV(2);

//Port funtions
#define PORT_ENCODER      PORTC

//Port direction registers
#define DDR_ENCODER      DDRC

//Port bitmasks
static const uint8_t kPinsEncoder      = 0x07;

//Encoder Pin Change Interrupts
#define PCINT_MASK_ENCODER PCMSK1
static const uint8_t kPCINTEnableEncoder = _BV(PCIE1);
static const uint8_t kPCINTMaskEncoder = _BV(PCINT8) | _BV(PCINT10);

//Global Pin Change Interrupt Tracking
static volatile uint8_t gPCINTPortLastValue = 0;

//Global Encoder Variables
static volatile uint8_t gEncoderValue = 0;
static volatile uint8_t gEncoderMin = 0;
static volatile uint8_t gEncoderMax = 0;
static volatile uint32_t gEncoderEnterStartTime = 0;
static volatile uint8_t gEncoderState = 0;

void encoder_init(void) {
  //Set pin directions
  DDR_ENCODER &= ~kPinsEncoder;
  
  //Enable Encoder Pin Change Interrupt
  PCICR |= kPCINTEnableEncoder;
  
  //Set Pin Change Interrupt Mask for EncA and Enter
  PCINT_MASK_ENCODER |= kPCINTMaskEncoder;
  
  //  Enable global interrupts 
  sei();
}

uint8_t encoder_get_value(void) {
  uint8_t encoderValue;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    encoderValue = gEncoderValue;
  } 
  return encoderValue;
}

ISR(PCINT1_vect) 
{
  
}