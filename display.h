#ifndef BOILPOWER_DISPLAY_H_
#define BOILPOWER_DISPLAY_H_

#include <stdint.h>
#include <avr/io.h> 

//Function declarations
void display_init(void);
void display_on(void);
void display_off(void);
void display_write_number(int number);
void display_write_string(const char* text);
void display_write_decimalpoint(uint8_t precision);
uint32_t millis(void);

#endif
