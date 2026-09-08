#pragma once

#include "esp_adc/adc_oneshot.h"

#define BAT GPIO_NUM_2

void adc_init();
void adc_read_raw(adc_channel_t chan, int *raw);
void adc_read_filtered(adc_channel_t chan, int *filtered);