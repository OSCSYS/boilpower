#include "ui.h"
#include "calcs.h"
#include "display.h"
#include "encoder.h"
#include "hwprofile.h"

typedef struct {
  char title[4];
  uint8_t (*menuFunc)(BoilPowerSettings*);
} menuItem;

uint8_t ui_period(BoilPowerSettings *settings);
uint8_t ui_sensitivity(BoilPowerSettings *settings);
uint8_t ui_frequency(BoilPowerSettings *settings);
uint8_t ui_user1(BoilPowerSettings *settings);
uint8_t ui_user2(BoilPowerSettings *settings);
uint8_t ui_user3(BoilPowerSettings *settings);
uint8_t ui_exit(BoilPowerSettings *settings);
uint16_t ui_get_value(uint16_t value, uint8_t minValue, uint8_t maxValue, uint8_t decimalPosition, uint16_t (*calc_function)(uint8_t, uint8_t));

static const menuItem kSettingsMenu[] = {
  {"Prd", ui_period},
  {"SEN", ui_sensitivity},
  {"Frq", ui_frequency},
  {" U1", ui_user1},
  {" U2", ui_user2},
  {" U3", ui_user3},
  {"SEt", ui_exit}
};

void ui_menu(BoilPowerSettings *settings) {
  enum {
    kMenuStateInit,
    kMenuStateUpdate,
    kMenuStateReady
  } menuState = kMenuStateInit;
  
  uint8_t menuPos = 0;
  
  while(1) {
    switch (menuState) {
      case kMenuStateInit:
        encoder_set_limits(0, sizeof(kSettingsMenu) / sizeof(kSettingsMenu[0]) - 1);
        encoder_set_value(menuPos);
        
      case kMenuStateUpdate:
        display_write_string(kSettingsMenu[menuPos].title);
        menuState = kMenuStateReady;
        
      case kMenuStateReady:
        if(encoder_changed()) {
          menuPos = encoder_value();
          menuState = kMenuStateUpdate;
        }
        if(encoder_ok()) {
          if(kSettingsMenu[menuPos].menuFunc(settings)) {
            //Exit Signal
            return;
          }
          menuState = kMenuStateInit;
        }
        if(encoder_cancel()) {
            return;
        }
    }
  }
}

uint8_t ui_period(BoilPowerSettings *settings) {
  settings->data.period = ui_get_value(settings->data.period, 1, 255, 1, 0);
  settings->data.sensitivity = calcs_minimum_sensitivity(settings->data.sensitivity, settings->data.period, settings->data.frequency);
  return 0;
}

uint8_t ui_sensitivity(BoilPowerSettings *settings) {
  settings->data.sensitivity = ui_get_value(settings->data.sensitivity, calcs_minimum_sensitivity(1, settings->data.period, settings->data.frequency), 255, 0, 0);
  return 0;
}

uint8_t ui_frequency(BoilPowerSettings *settings) {
  settings->data.frequency = ui_get_value(settings->data.frequency, 1, 255, 0, 0);
  settings->data.sensitivity = calcs_minimum_sensitivity(settings->data.sensitivity, settings->data.period, settings->data.frequency);
  return 0;
}

uint8_t ui_user1(BoilPowerSettings *settings) {
  settings->data.userSetpoint[0] = ui_get_value(settings->data.userSetpoint[0], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_user2(BoilPowerSettings *settings) {
  settings->data.userSetpoint[1] = ui_get_value(settings->data.userSetpoint[1], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_user3(BoilPowerSettings *settings) {
  settings->data.userSetpoint[2] = ui_get_value(settings->data.userSetpoint[2], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_exit(BoilPowerSettings *settings) {
  return 1;
}

uint16_t ui_get_value(uint16_t value, uint8_t minValue, uint8_t maxValue, uint8_t decimalPosition, uint16_t (*calc_function)(uint8_t, uint8_t)) {
  encoder_set_limits(minValue, maxValue);
  encoder_set_value(value);
  uint16_t displayValue = 0, workingValue = 0;
  uint8_t updateRequired = 1;
  
  while (1) {
    if (encoder_changed()) {
      updateRequired = 1;
    }
    
    if (updateRequired) {
      workingValue = encoder_value();
      displayValue = calc_function ? (*calc_function)(workingValue, maxValue) : workingValue;
      if (displayValue > DISPLAY_MAX_NUMBER) {
        display_write_string(" ON");
        display_write_decimalpoint(0);
      } else {
        display_write_number(displayValue);
        display_write_decimalpoint(decimalPosition + 1);
      }
      updateRequired = 0;
    }
    
    if(encoder_ok()) {
      return workingValue;
    }
    if(encoder_cancel()) {
      return value;
    }
  }
}