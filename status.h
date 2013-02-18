#ifndef BOILTROLLER_STATUS_H_
#define BOILTROLLER_STATUS_H_

#include <stdint.h>
#include <avr/io.h> 
 
//Status port bit mapping
static const uint8_t kStatusHeat   = _BV(0);
static const uint8_t kStatusLock = _BV(1);
static const uint8_t kStatusDebug = _BV(6);
 
//Function Declarations
void status_init(void);
void status_set(int status_mode);

#endif