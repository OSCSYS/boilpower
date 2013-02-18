#include "status.h"

static const uint8_t kPinsStatus      = 0x23;

//Port funtions
#define PORT_STATUS PORTB
#define DDR_STATUS DDRB

void status_init(void) {
  //Set pin directions
  DDR_STATUS |= kPinsStatus;
}

void status_set(int status_mode) {
  PORT_STATUS = (PORT_STATUS & (~kPinsStatus)) | status_mode;
}