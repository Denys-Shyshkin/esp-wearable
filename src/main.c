#include "button/button.h"
#include "display/display.h"
#include "display/graphics.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display/font_8x8.h"

#define BUTTON_UP GPIO_NUM_7
#define BUTTON_DOWN GPIO_NUM_8

#define SUPERLOOP_DELAY 10

static const char *TAG = "MAIN";

#define DEFAULT_BUTTON (Button){.gpio = -1, .btn_state = 1, .last_btn_state = 1, .s_last_btn_pressed = 0, .is_btn_pressed = 0}

Button btn_up = DEFAULT_BUTTON;
Button btn_down = DEFAULT_BUTTON;

enum Screen {
    STARTUP,
    TIME,
    WEATHER,
    ALERTS,
    HEART,
    TOTAL_COUNT,
};

enum Screen screen_number = WEATHER;
enum Screen last_screen_number = TOTAL_COUNT;

const uint8_t MAX_SCREENS_QTY = TOTAL_COUNT - 1;

enum Screen_Event {
    ENTER,
    UPDATE,
};

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

static void startup_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();
    }

    const char *text = "Startup";
    gfx_draw_text(0, 20, text, WHITE_COLOR, 2);
}

static void time_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        // gfx_draw_alignment_lines();

        const char *bat_percentage = "78%";
        gfx_draw_text(98, 0, bat_percentage, WHITE_COLOR, 2);

        const char *date = "July 27";
        gfx_draw_text(0, 70, date, WHITE_COLOR, 2);

        const char *seconds = "43";
        gfx_draw_text(210, 70, seconds, WHITE_COLOR, 2);

        const char *hours = "20";
        const char *separator = ":";
        const char *mins = "56";
        gfx_draw_text(0, 90, hours, WHITE_COLOR, 7);
        gfx_draw_text(105, 95, separator, WHITE_COLOR, 5);
        gfx_draw_text(142, 90, mins, WHITE_COLOR, 7);

        const char *steps_count = "1234";
        gfx_draw_text(90, 225, steps_count, WHITE_COLOR, 2);
    }
}

static void weather_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        // gfx_draw_alignment_lines();

        const char *location_name = "Kyiv";
        gfx_draw_text(90, 0, location_name, WHITE_COLOR, 2);

        gfx_draw_line(60, 30, 180, 30, WHITE_COLOR);

        const char *temp_value = "24";
        gfx_draw_text(100, 60, temp_value, WHITE_COLOR, 5);

        gfx_draw_spec_char(170, 60, degree_char, WHITE_COLOR, 4);

        const char *temp_unit = "C";
        gfx_draw_text(200, 60, temp_unit, WHITE_COLOR, 5);

        gfx_draw_line(20, 120, 220, 120, WHITE_COLOR);

        const char *wind_value = "2.1 km/h";
        gfx_draw_text(100, 140, wind_value, WHITE_COLOR, 2);

        const char *pressure_value = "1015";
        gfx_draw_text(100, 170, pressure_value, WHITE_COLOR, 2);

        const char *humidity_value = "27%";
        gfx_draw_text(100, 200, humidity_value, WHITE_COLOR, 2);
    }
}

static void alerts_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();
    }

    const char *text = "Alerts";
    gfx_draw_text(0, 20, text, WHITE_COLOR, 2);
}

static void heart_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();
    }

    const char *text = "Heart";
    gfx_draw_text(0, 20, text, WHITE_COLOR, 2);
}

static void screen_manager() {
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
        time_screen(event);
        break;

    case WEATHER:
        weather_screen(event);
        break;

    case ALERTS:
        alerts_screen(event);
        break;

    case HEART:
        heart_screen(event);
        break;

    case TOTAL_COUNT:
        break;
    }
}

void app_main() {
    display_init();

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