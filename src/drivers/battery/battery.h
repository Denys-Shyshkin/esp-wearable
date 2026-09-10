#pragma once

#include <stdint.h>

void battery_charge_pin_init();
bool battery_is_charging();
void battery_read_percentage(uint8_t *percentage);