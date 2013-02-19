#include "display.h"
#include "encoder.h"
#include "status.h"

int main(void) {
  status_init();
  display_init();
  encoder_init();

  if (encoder_raw_enter()) {
    settings();
  }

  encoder_set_limits(1,5);
  
  while (1) {
    if(encoder_changed()) {
      display_write_number(encoder_value());
    }
    if(encoder_ok()) {
      status_set(kStatusHeat);
    }
    if(encoder_cancel()) {
      status_clear(kStatusHeat);
    }
  }
}

void settings(void) {
  display_write_string("SET");
  while(!encoder_ok());
}