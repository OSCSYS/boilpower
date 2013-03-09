#ifndef BOILPOWER_SETTINGS_H_
#define BOILPOWER_SETTINGS_H_

#include <stdint.h>

static const uint8_t kSettingsVersion = 1;

typedef struct {
  uint8_t version;
  uint8_t size;
  uint8_t crc;
} BoilPowerSettingsHeader;

typedef struct {
  uint8_t period;           //Time in tenths of seconds of the entire period
  uint8_t sensitivity;      //Number of cycles per encoder click
  uint8_t frequency;        //Frequency in Hz of a single pulse
  uint8_t userSetpoint[3];  //User defined setpoints (0 = disabled)
} BoilPowerSettingsData;

typedef struct {
  BoilPowerSettingsHeader header;
  BoilPowerSettingsData data;
} BoilPowerSettings;

//Sets default values and returns 1 if settings are invalid
uint8_t settings_init(BoilPowerSettings *settings);
void settings_load(BoilPowerSettings *settings);
void settings_save(BoilPowerSettings *settings);

#endif