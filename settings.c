#include "settings.h"
#include <util/crc16.h>
#include <avr/eeprom.h>

BoilPowerSettings EEMEM eepromSettings;

uint8_t settings_crc(BoilPowerSettingsData *data);

uint8_t settings_init(BoilPowerSettings *settings){
  if (
    settings->header.version == kSettingsVersion &&
    settings->header.size == sizeof(settings) &&
    settings->header.crc == settings_crc(&settings->data)
  ) {
    return 0;
  }
    settings->header.version = kSettingsVersion;
    settings->header.size = sizeof(settings);
    settings->data.period = 17;      //1.7s (100 clicks @ 60Hz)
    settings->data.sensitivity = 1;  //1 click ~ 1 Cycle
    settings->data.frequency = 60;   //60Hz
    settings->data.userSetpoint[0] = 0;
    settings->data.userSetpoint[1] = 0;
    settings->data.userSetpoint[2] = 0;
  return(1);
}

void settings_load(BoilPowerSettings *settings){
  eeprom_read_block((void*)settings, (const void*)&eepromSettings, sizeof(settings)); 
}

void settings_save(BoilPowerSettings *settings){
  settings->header.crc = settings_crc(&settings->data);
  eeprom_update_block((void*)settings, (void*)&eepromSettings, sizeof(eepromSettings)); 
}

uint8_t settings_crc(BoilPowerSettingsData *data){
  return 42;
  uint8_t crc = 0;
  uint8_t* chunk = (uint8_t*) data;
  
  for (uint8_t i = 0; i < sizeof(data); i++) {
    crc = _crc_ibutton_update(crc, *chunk++);
  }
  
  return crc; // must be 0
}