#include "encoder.h"

#include <avr/interrupt.h>
#include <util/atomic.h> 

#include "display.h" //Provides millis()
#include "hwprofile.h"

static const uint16_t kEncoderOKDuration = 50;
static const uint16_t kEncoderCancelDuration = 1000;

//Global Encoder Variables
static volatile int gEncoderValue = 0;
static volatile uint8_t gEncoderMin = 0;
static volatile uint8_t gEncoderMax = 0;
static volatile uint8_t gEncoderChanged = 0;
static volatile uint32_t gEncoderEnterStartTime = 0;
static volatile EncoderEnterState gEncoderEnterState = kEncoderEnterStateIdle;
static volatile uint8_t gEncoderLastBits = 0;

void encoder_init(void) {
  //Set pin directions
  ENCODER_DIR_REG &= ~kEncoderPinMask;
  
  //Enable Encoder Pin Change Interrupt
  PCICR |= kEncoderPCINTPort;
  
  //Set Pin Change Interrupt Mask for EncA and Enter
  ENCODER_PCINT_MASK_REG |= kEncoderPCINTMask;
  
  //  Enable global interrupts 
  sei();
}

int encoder_value(void) {
  int encoderValue;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    encoderValue = gEncoderValue;
    gEncoderChanged = 0;
  } 
  return encoderValue;
}

uint8_t encoder_changed(void) {
  uint8_t changed;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  {
    changed = gEncoderChanged;
  }
  return changed;
}

void encoder_set_limits(uint8_t minimum, uint8_t maximum) {
  gEncoderMin = minimum;
  gEncoderMax = maximum;
  //Reset value to ensure within limits
  uint8_t value = encoder_value();
  encoder_set_value(value);
}

void encoder_set_value(int value) {
  value = value > gEncoderMax ? gEncoderMax : value;
  value = value < gEncoderMin ? gEncoderMin : value;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    gEncoderValue = value;
  } 
}

uint8_t encoder_ok(void) {
  uint8_t isOK = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  { 
    if(gEncoderEnterState==kEncoderEnterStateOK) {
      gEncoderEnterState = kEncoderEnterStateIdle;
      isOK = 1;
    }
  }
  return isOK;
}

uint8_t encoder_cancel(void){
  uint8_t isCancel = 0;
  uint32_t timestamp = millis();
  timestamp = (timestamp > kEncoderCancelDuration) ? (timestamp - kEncoderCancelDuration) : 0;
  
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  {
    if(gEncoderEnterState==kEncoderEnterStateClicked) {
      if(gEncoderEnterStartTime < timestamp) {
        gEncoderEnterState = kEncoderEnterStateCancel;
      }
    }
  }
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
  {
    if(gEncoderEnterState==kEncoderEnterStateCancel) {
      gEncoderEnterState = kEncoderEnterStateIdle;
      isCancel = 1;
    }
  }
  return isCancel;
}

uint8_t encoder_raw_enter(void) {
  return !(ENCODER_INPUT_REG & kEncoderPinE);
}

ISR(ENCODER_PCINT_VECTOR) 
{
  uint8_t encoderBits = ENCODER_INPUT_REG;
  uint8_t encoderChangedBits = gEncoderLastBits ^ encoderBits;

  //Process Encoder A Pin Change
  if (encoderChangedBits & kEncoderPinA) {
    //Trigger on rising
    if ((encoderBits & kEncoderPinA)) {
      if(encoderBits & kEncoderPinB) {
        if (gEncoderValue > gEncoderMin) {
          --gEncoderValue;
        }
      } else {
        if (gEncoderValue < gEncoderMax) {
          ++gEncoderValue;
        }
      }
      gEncoderChanged = 1; //Flag value as changed
    }
  }
  
  //Process Enter Pin Change
  if (encoderChangedBits & kEncoderPinE) {
    uint32_t time = millis();
   
    switch (gEncoderEnterState) {
      case kEncoderEnterStateIdle:
        //Button is pushed (ActiveLow)
        if (!(encoderBits & kEncoderPinE)) {    
          gEncoderEnterState = kEncoderEnterStateClicked;
          gEncoderEnterStartTime = time;
        }
        break;
      case kEncoderEnterStateClicked:
        //Assumes interrupt must be enter release
        if (time > gEncoderEnterStartTime + kEncoderCancelDuration) {
          //Long click is a Cancel
          gEncoderEnterState = kEncoderEnterStateCancel;
        } else if (time > gEncoderEnterStartTime + kEncoderOKDuration) {
          //Debounce Enter using minimum threshold
          gEncoderEnterState = kEncoderEnterStateOK;
        } else {
          //Didn't meet minimum, back to idle
          gEncoderEnterState = kEncoderEnterStateIdle;
        }
        break;
      default:
        //Events in OK/Cancel state ignored
        break;
    }
  }
  gEncoderLastBits = encoderBits;
}