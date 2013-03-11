#ifndef BOILPOWER_UI_H_
#define BOILPOWER_UI_H_

#include "settings.h"

void ui_settings_menu(struct BoilPowerSettings *settings);
void ui_init(const struct BoilPowerSettings *settings);
void ui_update(void);

#endif