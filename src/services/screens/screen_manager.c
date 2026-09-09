#include "screen_manager.h"
#include "config/config.h"
#include "esp_sleep.h"
#include "heart_screen.h"
#include "startup_screen.h"
#include "time_screen.h"
#include "weather_screen.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SLEEP_MODE_ON

enum Screen screen_number = STARTUP;
static enum Screen last_screen_number = STARTUP;

const uint8_t MAX_SCREENS_QTY = TOTAL_COUNT - 1;

void go_screen_up() {
    if (screen_number >= MAX_SCREENS_QTY) {
        screen_number = 1;
    } else {
        screen_number++;
    }
}

void go_screen_down() {
    if (screen_number <= 1) {
        screen_number = MAX_SCREENS_QTY;
    } else {
        screen_number--;
    }
}

void screen_change(button *btn_up, button *btn_down) {
    if (btn_up->is_btn_pressed) {
        btn_up->is_btn_pressed = 0;

        go_screen_up();
    }

    if (btn_down->is_btn_pressed) {
        btn_down->is_btn_pressed = 0;

        go_screen_down();
    }
}

void screen_light_sleep(uint64_t sleep_time) {
#ifdef SLEEP_MODE_ON
    esp_sleep_enable_timer_wakeup(sleep_time);
    esp_light_sleep_start();

    uint32_t wakeup_causes = esp_sleep_get_wakeup_causes();

    if (wakeup_causes & (1UL << ESP_SLEEP_WAKEUP_GPIO)) {
        if (gpio_get_level(PIN_BUTTON_UP) == 0) {
            go_screen_up();
        }

        if (gpio_get_level(PIN_BUTTON_DOWN) == 0) {
            go_screen_down();
        }
    }
#endif
}

void screen_manager(imu_sensor *imu, hr_sensor *hr) {
    enum Screen_Event event = UPDATE;

    if (screen_number != last_screen_number) {
        event = ENTER;
        last_screen_number = screen_number;
    }

    switch (screen_number) {
    case STARTUP:
        startup_screen(event);
        break;

    case TIME:
        time_screen(event, imu);
        break;

    case WEATHER:
        weather_screen(event);
        break;

    case HEART:
        heart_screen(event, hr);
        break;

    case TOTAL_COUNT:
        break;
    }
}