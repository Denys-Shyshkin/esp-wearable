#pragma once

#include <stdint.h>

extern volatile bool battery_is_charging;

void battery_charge_pin_init();
void battery_read_percentage(uint8_t *percentage);