#include "config/config.h"
#include "drivers/adc/adc.h"
#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"
#include "services/time/time.h"
#include "esp_timer.h"

// #define SHOW_BAT_VOLTAGE

#define MAX_BAT_RAW_VALUE 4096
#define MAX_BAT_VOLTAGE 4.2
#define MIN_BAT_VOLTAGE 3.0
#define TIME_SCREEN_SLEEP_TIMEOUT_US 1 * 1000 * 1000
#define BAT_PERCENTAGE_UPDATE_INTERVAL_US 1 * 60 * 1000 * 1000

static void draw_bat_percentage(enum Screen_Event event) {
    static uint8_t displayed_percentage;
    static uint64_t last_update = 0;

    static bool charging_state_displayed = false;

    bool is_charging = gpio_get_level(PIN_CHARGE) == 1;
    int filtered = 0;
    adc_read_filtered(PIN_BAT, &filtered);
    float voltage = (float)filtered * (MAX_BAT_VOLTAGE / MAX_BAT_RAW_VALUE);
    uint8_t percentage = ((voltage - MIN_BAT_VOLTAGE) / (MAX_BAT_VOLTAGE - MIN_BAT_VOLTAGE)) * 100;

    if (percentage > 95) {
        percentage = 100;
    } else if (percentage < 5) {
        percentage = 5;
    } else if (percentage <= 95 || percentage >= 5) {
        percentage = (percentage / 5) * 5;
    }

    uint64_t now = esp_timer_get_time();

    if ((((displayed_percentage > percentage && !is_charging) || (displayed_percentage < percentage && is_charging)) && now - last_update >= BAT_PERCENTAGE_UPDATE_INTERVAL_US) || event == ENTER) {
        gfx_fill_rect(98, 10, 70, 15, BLACK_COLOR); // clear percentage
        char percentage_buffer[10];
        snprintf(percentage_buffer, sizeof(percentage_buffer), "%d%%", percentage);
        gfx_draw_text(98, 10, percentage_buffer, WHITE_COLOR, 2);

#ifdef SHOW_BAT_VOLTAGE
        gfx_fill_rect(98, 25, 70, 15, BLACK_COLOR);
        char voltage_buffer[10];
        snprintf(voltage_buffer, sizeof(voltage_buffer), "%.2f", voltage);
        gfx_draw_text(98, 25, voltage_buffer, WHITE_COLOR, 2);
#endif

        displayed_percentage = percentage;
        last_update = now;
    }

    if (charging_state_displayed != is_charging || event == ENTER) {
        if (is_charging) {
            gfx_draw_icon(60, 0, charging, GREEN_COLOR, 1);
        } else {
            gfx_fill_rect(60, 0, 30, 40, BLACK_COLOR);
        }

        charging_state_displayed = is_charging;
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
        gfx_fill_rect(135, 90, 105, 50, BLACK_COLOR); // clear mins
        char mins_buff[6];
        strftime(mins_buff, sizeof(mins_buff), "%M", timeinfo);
        gfx_draw_text(135, 90, mins_buff, WHITE_COLOR, 7);

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

static uint8_t get_steps_count_position(uint32_t steps) {
    if (steps > 9999)
        return 80;
    else if (steps > 999)
        return 90;
    else if (steps > 99)
        return 95;
    else if (steps > 9)
        return 105;
    else
        return 115;
}

static void draw_steps_count(enum Screen_Event event, imu_sensor *imu) {
    static uint32_t displayed_steps = 0;

    uint32_t steps_count;
    imu_read_steps(imu, &steps_count);

    if (displayed_steps != steps_count || event == ENTER) {
        gfx_fill_rect(70, 225, 100, 30, BLACK_COLOR); // clear steps
        char steps_buff[6];
        snprintf(steps_buff, sizeof(steps_buff), "%ld", steps_count);
        gfx_draw_text(get_steps_count_position(steps_count), 225, steps_buff, WHITE_COLOR, 2);

        displayed_steps = steps_count;
    }
}

void time_screen(enum Screen_Event event, imu_sensor *imu) {
    struct tm timeinfo;
    get_time(&timeinfo);

    if (event == ENTER) {
        display_clear();

        const char *time_separator = ":";
        gfx_draw_text(103, 95, time_separator, WHITE_COLOR, 5);
    }

    draw_bat_percentage(event);

    draw_seconds(event, &timeinfo);
    draw_hours_minutes(event, &timeinfo);
    draw_date(event, &timeinfo);

    draw_steps_count(event, imu);

    screen_light_sleep(TIME_SCREEN_SLEEP_TIMEOUT_US);
}