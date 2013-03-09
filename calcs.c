#include "calcs.h"

uint8_t calcs_minimum_sensitivity(uint8_t sensitivity, uint8_t period, uint8_t frequency) {
  uint8_t minimumSensitivity = period * frequency / 255 / 10;
  return sensitivity < minimumSensitivity ? minimumSensitivity : sensitivity;
}

uint8_t calcs_range(uint8_t period, uint8_t frequency, uint8_t sensitivity) {
  return period * frequency / sensitivity / 10;
}

uint16_t calcs_pwm_time(uint8_t period, uint8_t value, uint8_t range) {
  return period * 100 * value / range;
}

uint16_t calcs_pwm_percent(uint8_t value, uint8_t range) {
  return (uint32_t) value * 1000 / range;
}