#ifndef BOILTROLLER_ENCODER_H_
#define BOILTROLLER_ENCODER_H_

#include <stdint.h>
#include <avr/io.h> 

typedef enum {
  kEncoderEnterStateIdle,
  kEncoderEnterStateClicked,
  kEncoderEnterStateOK,
  kEncoderEnterStateCancel
} EncoderEnterState;
  
//Function declarations
void encoder_init(void);
void encoder_set_limits(uint8_t minimum, uint8_t maximum);
int encoder_value(void);
uint8_t encoder_changed(void);
void encoder_set_value(int value);
uint8_t encoder_ok(void);
uint8_t encoder_cancel(void);
uint8_t encoder_raw_enter(void);

#endif