#include "ui.h"

#include <stdint.h>
#include <string.h>

#include "calcs.h"
#include "display.h"
#include "encoder.h"
#include "hwprofile.h"
#include "pwm.h"
#include "status.h"

uint8_t ui_period(struct BoilPowerSettings *settings);
uint8_t ui_sensitivity(struct BoilPowerSettings *settings);
uint8_t ui_frequency(struct BoilPowerSettings *settings);
uint8_t ui_user1(struct BoilPowerSettings *settings);
uint8_t ui_user2(struct BoilPowerSettings *settings);
uint8_t ui_user3(struct BoilPowerSettings *settings);
uint8_t ui_exit(struct BoilPowerSettings *settings);
uint16_t ui_get_value(uint16_t value, uint8_t minValue, uint8_t maxValue, uint8_t decimalPosition, uint16_t (*calc_function)(uint8_t, uint8_t));

static uint8_t gUiSetpoints[5] = {0, 0, 0, 0, 0};

void ui_init(const struct BoilPowerSettings *settings)
{
  encoder_set_limits(0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity));
  encoder_set_value(0);
  
  gUiSetpoints[1] = settings->data.userSetpoint[0];
  gUiSetpoints[2] = settings->data.userSetpoint[1];
  gUiSetpoints[3] = settings->data.userSetpoint[2];
  gUiSetpoints[4] = encoder_maximum();
  
  display_write_string("Off");
  status_set(kStatusLock);
}

void ui_update()
{
  static enum {
    kUiStateLocked,
    kUiStateOff,
    kUiStateU1,
    kUiStateU2,
    kUiStateU3,
    kUiStateOn,
    kUiStateNumStates
  } uiState = kUiStateLocked;
  
  if (uiState == kUiStateLocked) {
    if (encoder_cancel()) {
      //Unlock
      uiState = kUiStateOff;
      status_clear(kStatusLock);
      encoder_set_value(0);
    } else if (encoder_ok()) {
      //Dummy check to clear Enter
    }
  } else {
    uint8_t updateRequired = 0;
    if (encoder_cancel()) {
      //Transition to Locked State
      uiState = kUiStateLocked;
      pwm_set_level(0);
      status_set(kStatusLock);
      display_write_string("Off");
    }
    if (encoder_ok()) {
      //Advance state
      uint8_t newSetpoint = 0;
      while (!newSetpoint) {
        ++uiState;
        if (uiState == kUiStateNumStates) {
          uiState = kUiStateOff;
          newSetpoint = 0;
          break;
        }
        newSetpoint = gUiSetpoints[uiState - kUiStateOff];
      }
      //Set Encoder value to setpoint associated with state
      encoder_set_value(newSetpoint);
      updateRequired = 1;
    }
    if(encoder_changed())
      updateRequired = 1;
    if(updateRequired) {
      uint8_t value = encoder_value();
      uint8_t maximum = encoder_maximum();
      if (!value)
        display_write_string("Off");
      else if (value == maximum)
        display_write_string(" On");
      else
        display_write_number(calcs_pwm_percent(value, maximum), 1);
      pwm_set_level(calcs_pwm_time(pwm_period(), value, maximum));
    }
  }
}

struct menuItem {
  char title[4];
  uint8_t (*menuFunc)(struct BoilPowerSettings*);
};

static const struct menuItem kSettingsMenu[] = {
  {"Prd", ui_period},
  {"SEN", ui_sensitivity},
  {"Frq", ui_frequency},
  {" U1", ui_user1},
  {" U2", ui_user2},
  {" U3", ui_user3},
  {"SEt", ui_exit}
};

void ui_settings_menu(struct BoilPowerSettings *settings)
{
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
          if(kSettingsMenu[menuPos].menuFunc(settings))
            return; //Exit Signal
          menuState = kMenuStateInit;
        }
        if(encoder_cancel())
            return;
    }
  }
}

uint8_t ui_period(struct BoilPowerSettings *settings)
{
  settings->data.period = ui_get_value(settings->data.period, 1, 255, 1, 0);
  settings->data.sensitivity = calcs_minimum_sensitivity(settings->data.sensitivity, settings->data.period, settings->data.frequency);
  return 0;
}

uint8_t ui_sensitivity(struct BoilPowerSettings *settings)
{
  settings->data.sensitivity = ui_get_value(settings->data.sensitivity, calcs_minimum_sensitivity(1, settings->data.period, settings->data.frequency), 255, 0, 0);
  return 0;
}

uint8_t ui_frequency(struct BoilPowerSettings *settings)
{
  settings->data.frequency = ui_get_value(settings->data.frequency, 1, 255, 0, 0);
  settings->data.sensitivity = calcs_minimum_sensitivity(settings->data.sensitivity, settings->data.period, settings->data.frequency);
  return 0;
}

uint8_t ui_user1(struct BoilPowerSettings *settings)
{
  settings->data.userSetpoint[0] = ui_get_value(settings->data.userSetpoint[0], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_user2(struct BoilPowerSettings *settings)
{
  settings->data.userSetpoint[1] = ui_get_value(settings->data.userSetpoint[1], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_user3(struct BoilPowerSettings *settings)
{
  settings->data.userSetpoint[2] = ui_get_value(settings->data.userSetpoint[2], 0, calcs_range(settings->data.period, settings->data.frequency, settings->data.sensitivity), 1, calcs_pwm_percent);
  return 0;
}

uint8_t ui_exit(struct BoilPowerSettings *settings)
{
  return 1;
}

uint16_t ui_get_value(uint16_t value, uint8_t minValue, uint8_t maxValue, uint8_t decimalPosition, uint16_t (*calc_function)(uint8_t, uint8_t))
{
  encoder_set_limits(minValue, maxValue);
  encoder_set_value(value);
  uint16_t displayValue = 0, workingValue = 0;
  uint8_t updateRequired = 1;
  
  while (1) {
    if (encoder_changed())
      updateRequired = 1;
    if (updateRequired) {
      workingValue = encoder_value();
      displayValue = calc_function ? (*calc_function)(workingValue, maxValue) : workingValue;
      if (displayValue > DISPLAY_MAX_NUMBER)
        display_write_string(" ON");
      else
        display_write_number(displayValue, decimalPosition);
      updateRequired = 0;
    }
    if(encoder_ok())
      return workingValue;
    if(encoder_cancel())
      return value;
  }
}
