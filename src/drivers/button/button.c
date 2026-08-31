#include "button.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_DEBOUNCE_US 20000

void button_init(button *btn, gpio_num_t gpio) {
    btn->gpio = gpio;

    gpio_config_t io_config = {
        .pin_bit_mask = 1ULL << btn->gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_config));
}

void button_is_pressed(button *btn) {
    uint32_t now = esp_timer_get_time();

    bool button_read = gpio_get_level(btn->gpio);

    if (button_read != btn->last_btn_state) {
        btn->s_last_btn_pressed = now;
    }

    if (now - btn->s_last_btn_pressed >= BUTTON_DEBOUNCE_US) {
        if (button_read != btn->btn_state) {
            btn->btn_state = button_read;

            if (button_read == 0) {
                btn->is_btn_pressed = 1;
            }
        }
    }

    btn->last_btn_state = button_read;
}