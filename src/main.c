#include "button/button.h"
#include "display/display.h"
#include "display/font_8x8.h"
#include "display/graphics.h"
#include "display/icons.h"
#include "screens/screens.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define BUTTON_UP GPIO_NUM_7
#define BUTTON_DOWN GPIO_NUM_8

#define SUPERLOOP_DELAY 10

static const char *TAG = "MAIN";

#define DEFAULT_BUTTON (Button){.gpio = -1, .btn_state = 1, .last_btn_state = 1, .s_last_btn_pressed = 0, .is_btn_pressed = 0}

Button btn_up = DEFAULT_BUTTON;
Button btn_down = DEFAULT_BUTTON;

static void buttons_reading() {
    button_is_pressed(&btn_up);
    button_is_pressed(&btn_down);
}

static void screen_change() {
    if (btn_up.is_btn_pressed) {
        btn_up.is_btn_pressed = 0;

        if (screen_number >= MAX_SCREENS_QTY) {
            screen_number = 1;
        } else {
            screen_number++;
        }
    }

    if (btn_down.is_btn_pressed) {
        btn_down.is_btn_pressed = 0;

        if (screen_number <= 1) {
            screen_number = MAX_SCREENS_QTY;
        } else {
            screen_number--;
        }
    }
}

void app_main() {
    display_init();
    loading_spinner_frames_init();

    button_init(&btn_up, BUTTON_UP);
    button_init(&btn_down, BUTTON_DOWN);

    while (1) {
        buttons_reading();
        screen_change();
        screen_manager();

        ESP_LOGI(TAG, "Current screen number: %d \n", screen_number);

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY));
    }
}