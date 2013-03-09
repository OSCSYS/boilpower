#include "display.h"
#include "encoder.h"
#include "settings.h"
#include "status.h"
#include "ui.h"

void settingsUI(BoilPowerSettings *settings);

int main(void) {
  status_init();
  display_init();
  encoder_init();

  BoilPowerSettings systemSettings;
  settings_load(&systemSettings);
  
  if (settings_init(&systemSettings) || encoder_raw_enter()) {
    ui_menu(&systemSettings);
    settings_save(&systemSettings);
  }
  
  while (1) {

  }
}

