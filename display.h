#ifndef BOILTROLLER_DISPLAY_H_
#define BOILTROLLER_DISPLAY_H_

#include <stdint.h>
#include <avr/io.h> 

//Function declarations
void display_init(void);
void display_on(void);
void display_off(void);
void display_write_number(int number);
void display_write_decimalpoint(uint8_t precision);
uint32_t display_millis(void);

#endif
