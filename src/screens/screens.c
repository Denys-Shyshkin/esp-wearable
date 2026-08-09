#include "screens.h"
#include "display/display.h"
#include "display/font_8x8.h"
#include "display/graphics.h"
#include "display/icons.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define HEART_DRAW_DELAY_US 400 * 1000
#define HEART_ANIM_FRAMES_QTY 2

Animation_Frame beating_heart[HEART_ANIM_FRAMES_QTY] = {
    {.x = 105, .y = 65, .icon = heart_icon, .color = RED_COLOR, .scale = 1},
    {.x = 90, .y = 50, .icon = heart_icon, .color = RED_COLOR, .scale = 2},
};

enum Screen screen_number = STARTUP;
enum Screen last_screen_number = STARTUP;

const uint8_t MAX_SCREENS_QTY = TOTAL_COUNT - 1;

void startup_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *startup = "STARTING";
        gfx_draw_text(60, 20, startup, SEA_GREEN_COLOR, 2);

        // const char *weather_update = "Weather update..";
        // gfx_draw_text(40, 120, weather_update, LIGHT_GREY_COLOR, 1);

        // const char *mui_init = "IMU init........";
        // gfx_draw_text(40, 140, mui_init, LIGHT_GREY_COLOR, 1);

        // const char *hr_sensor = "HR sensor.......";
        // gfx_draw_text(40, 160, hr_sensor, LIGHT_GREY_COLOR, 1);
    }
}

void time_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

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

void weather_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *location_name = "Kyiv";
        gfx_draw_text(90, 0, location_name, WHITE_COLOR, 2);

        gfx_draw_line(60, 30, 180, 30, WHITE_COLOR);

        gfx_draw_icon(10, 45, weather_01d, WHITE_COLOR, 2);

        const char *temp_value = "24";
        gfx_draw_text(80, 60, temp_value, WHITE_COLOR, 5);

        gfx_draw_spec_char(160, 60, degree_char, WHITE_COLOR, 4);

        const char *temp_unit = "C";
        gfx_draw_text(190, 60, temp_unit, WHITE_COLOR, 5);

        gfx_draw_line(20, 120, 220, 120, WHITE_COLOR);

        gfx_draw_icon(30, 130, wind_icon, LIGHT_BLUE_COLOR, 1);
        gfx_draw_icon(30, 170, pressure_icon, LIGHT_BLUE_COLOR, 1);
        gfx_draw_icon(30, 210, humidity_icon, LIGHT_BLUE_COLOR, 1);

        const char *wind_value = "2.1 km/h";
        gfx_draw_text(80, 140, wind_value, LIGHT_GREY_COLOR, 2);

        const char *pressure_value = "1015 mbar";
        gfx_draw_text(80, 180, pressure_value, LIGHT_GREY_COLOR, 2);

        const char *humidity_value = "27 %";
        gfx_draw_text(80, 220, humidity_value, LIGHT_GREY_COLOR, 2);
    }
}

void heart_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *screen_name = "Heart rate";
        gfx_draw_text(35, 0, screen_name, WHITE_COLOR, 2);

        const char *heart_rate = "62";
        gfx_draw_text(85, 140, heart_rate, WHITE_COLOR, 5);
    }

    gfx_animation(90, 50, 65, 60, beating_heart, HEART_ANIM_FRAMES_QTY, HEART_DRAW_DELAY_US);
}

void screen_manager() {
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

    case HEART:
        heart_screen(event);
        break;

    case TOTAL_COUNT:
        break;
    }
}