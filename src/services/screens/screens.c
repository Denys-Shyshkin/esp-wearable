#include "screens.h"
#include "drivers/display/display.h"
#include "services/graphics/font_8x8.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"
#include "services/parser/weather.h"
#include "services/time/time.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

#define HEART_DRAW_DELAY_US 400 * 1000
#define HEART_ANIM_FRAMES_QTY 2

static const char *TAG = "SCREENS";

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

        // const char *mui_init = "IMU init........";
        // gfx_draw_text(40, 140, mui_init, LIGHT_GREY_COLOR, 1);

        // const char *hr_sensor = "HR sensor.......";
        // gfx_draw_text(40, 160, hr_sensor, LIGHT_GREY_COLOR, 1);
    }
}

static void draw_seconds(enum Screen_Event event, struct tm *timeinfo) {
    static uint8_t displayed_seconds;

    if (displayed_seconds != timeinfo->tm_sec || event == ENTER) {
        gfx_fill_rect(210, 70, 30, 15, BLACK_COLOR); // clear seconds
        char sec_buff[3];
        strftime(sec_buff, sizeof(sec_buff), "%S", timeinfo);
        gfx_draw_text(210, 70, sec_buff, WHITE_COLOR, 2);

        displayed_seconds = timeinfo->tm_sec;
    }
}

static void draw_hours_minutes(enum Screen_Event event, struct tm *timeinfo) {
    static uint8_t displayed_minutes;
    static uint8_t displayed_hours;

    if (displayed_minutes != timeinfo->tm_min || event == ENTER) {
        gfx_fill_rect(142, 90, 100, 50, BLACK_COLOR); // clear mins
        char mins_buff[6];
        strftime(mins_buff, sizeof(mins_buff), "%M", timeinfo);
        gfx_draw_text(142, 90, mins_buff, WHITE_COLOR, 7);

        displayed_minutes = timeinfo->tm_min;
    }

    if (displayed_hours != timeinfo->tm_hour || event == ENTER) {
        gfx_fill_rect(0, 90, 105, 50, BLACK_COLOR); // clear hours
        char hour_buff[6];
        strftime(hour_buff, sizeof(hour_buff), "%H", timeinfo);
        gfx_draw_text(0, 90, hour_buff, WHITE_COLOR, 7);

        displayed_hours = timeinfo->tm_hour;
    }
}

static void draw_date(enum Screen_Event event, struct tm *timeinfo) {
    static uint8_t displayed_date;

    if (displayed_date != timeinfo->tm_mday || event == ENTER) {
        gfx_fill_rect(0, 60, 200, 30, BLACK_COLOR); // clear date
        char date_buff[20];
        strftime(date_buff, sizeof(date_buff), "%a, %b %d", timeinfo);
        gfx_draw_text(0, 70, date_buff, WHITE_COLOR, 2);

        displayed_date = timeinfo->tm_mday;
    }
}

void time_screen(enum Screen_Event event) {
    struct tm timeinfo;
    get_time(&timeinfo);

    if (event == ENTER) {
        display_clear();

        const char *bat_percentage = "78%";
        gfx_draw_text(98, 0, bat_percentage, WHITE_COLOR, 2);

        const char *time_separator = ":";
        gfx_draw_text(105, 95, time_separator, WHITE_COLOR, 5);

        const char *steps_count = "1234";
        gfx_draw_text(90, 225, steps_count, WHITE_COLOR, 2);
    }

    draw_seconds(event, &timeinfo);
    draw_hours_minutes(event, &timeinfo);
    draw_date(event, &timeinfo);
}

static void draw_weather_icon(enum Screen_Event event) {
    static char *displayed_icon;

    if (displayed_icon != weather.icon || event == ENTER) {
        gfx_fill_rect(10, 45, 60, 60, BLACK_COLOR); // clear icon
        const uint32_t *icon = get_weather_icon(weather.icon);

        if (icon != NULL) {
            gfx_draw_icon(10, 45, icon, WHITE_COLOR, 2);
        } else {
            ESP_LOGE(TAG, "Icon not found for ID: %s", weather.icon);
        }

        displayed_icon = weather.icon;
    }
}

static void draw_temperature(enum Screen_Event event) {
    static uint8_t displayed_temp;

    if (displayed_temp != weather.temperature || event == ENTER) {
        gfx_fill_rect(80, 60, 80, 50, BLACK_COLOR); // clear temp
        char temp_buffer[5];
        snprintf(temp_buffer, sizeof(temp_buffer), "%d", weather.temperature);
        gfx_draw_text(80, 60, temp_buffer, WHITE_COLOR, 5);

        displayed_temp = weather.temperature;
    }
}

static void draw_wind(enum Screen_Event event) {
    static float displayed_wind;

    if (displayed_wind != weather.wind_speed || event == ENTER) {
        gfx_fill_rect(80, 140, 160, 20, BLACK_COLOR); // clear wind
        char wind_buffer[15];
        snprintf(wind_buffer, sizeof(wind_buffer), "%.1f km/h", weather.wind_speed);
        gfx_draw_text(80, 140, wind_buffer, LIGHT_GREY_COLOR, 2);

        displayed_wind = weather.wind_speed;
    }
}

static void draw_pressure(enum Screen_Event event) {
    static uint16_t displayed_pressure;

    if (displayed_pressure != weather.pressure || event == ENTER) {
        gfx_fill_rect(80, 180, 160, 20, BLACK_COLOR); // clear pressure
        char pressure_buffer[15];
        snprintf(pressure_buffer, sizeof(pressure_buffer), "%d mbar", weather.pressure);
        gfx_draw_text(80, 180, pressure_buffer, LIGHT_GREY_COLOR, 2);

        displayed_pressure = weather.pressure;
    }
}

static void draw_humidity(enum Screen_Event event) {
    static uint8_t displayed_humidity;

    if (displayed_humidity != weather.humidity || event == ENTER) {
        gfx_fill_rect(80, 220, 160, 20, BLACK_COLOR); // clear humidity
        char humidity_buffer[10];
        snprintf(humidity_buffer, sizeof(humidity_buffer), "%d %%", weather.humidity);
        gfx_draw_text(80, 220, humidity_buffer, LIGHT_GREY_COLOR, 2);

        displayed_humidity = weather.humidity;
    }
}

void weather_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *location_name = "Kyiv";
        gfx_draw_text(90, 0, location_name, WHITE_COLOR, 2);

        gfx_draw_line(60, 30, 180, 30, WHITE_COLOR);

        gfx_draw_spec_char(160, 60, degree_char, WHITE_COLOR, 4);

        const char *temp_unit = "C";
        gfx_draw_text(190, 60, temp_unit, WHITE_COLOR, 5);

        gfx_draw_line(20, 120, 220, 120, WHITE_COLOR);

        gfx_draw_icon(30, 130, wind_icon, LIGHT_BLUE_COLOR, 1);
        gfx_draw_icon(30, 170, pressure_icon, LIGHT_BLUE_COLOR, 1);
        gfx_draw_icon(30, 210, humidity_icon, LIGHT_BLUE_COLOR, 1);
    }

    draw_weather_icon(event);
    draw_temperature(event);
    draw_wind(event);
    draw_pressure(event);
    draw_humidity(event);
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