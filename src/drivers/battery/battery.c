#include "battery.h"
#include "config/config.h"
#include "driver/gpio.h"
#include "drivers/adc/adc.h"

#define MAX_BAT_RAW_VALUE 4096
#define MAX_BAT_VOLTAGE 4.2
#define MIN_BAT_VOLTAGE 3.0

void battery_charge_pin_init() {
    gpio_config_t io_config = {
        .pin_bit_mask = 1ULL << PIN_CHARGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_config));
}

bool battery_is_charging() {
    return gpio_get_level(PIN_CHARGE) == 1;
}

void battery_read_percentage(uint8_t *percentage) {
    int filtered = 0;
    adc_read_filtered(PIN_BAT, &filtered);

    float voltage = (float)filtered * (MAX_BAT_VOLTAGE / MAX_BAT_RAW_VALUE);

    *percentage = ((voltage - MIN_BAT_VOLTAGE) / (MAX_BAT_VOLTAGE - MIN_BAT_VOLTAGE)) * 100;

    if (*percentage > 95) {
        *percentage = 100;
    } else if (*percentage < 5) {
        *percentage = 5;
    } else if (*percentage <= 95 || *percentage >= 5) {
        *percentage = (*percentage / 5) * 5;
    }
}