#ifndef BOILTROLLER_ENCODER_H_
#define BOILTROLLER_ENCODER_H_

#include <stdint.h>
#include <avr/io.h> 

//Function declarations
void encoder_init(void);
uint8_t encoder_get_value(void);

#endif