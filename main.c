#include "display.h"
#include "encoder.h"
#include "status.h"

int main(void) {
  display_init();
  encoder_init();
  status_init();
  
  while (1) {
    display_write_number(display_millis() / 1000);
  }
}
