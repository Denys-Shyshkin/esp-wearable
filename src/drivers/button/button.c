#include "button.h"
#include "config/config.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_DEBOUNCE_US 20000

static void button_init(button *btn, gpio_num_t gpio) {
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

void buttons_init(button *btn_up, button *btn_down) {
    button_init(btn_up, PIN_BUTTON_UP);
    button_init(btn_down, PIN_BUTTON_DOWN);
}

static void button_is_pressed(button *btn) {
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

void buttons_reading(button *btn_up, button *btn_down) {
    button_is_pressed(btn_up);
    button_is_pressed(btn_down);
}