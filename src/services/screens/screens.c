#include "screens.h"
#include "config/config.h"
#include "drivers/adc/adc.h"
#include "drivers/button/button.h"
#include "drivers/display/display.h"
#include "drivers/hr/hr.h"
#include "drivers/imu/imu.h"
#include "esp_sleep.h"
#include "esp_timer.h"
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

#define TIME_SCREEN_SLEEP_TIMEOUT_US 1 * 1000 * 1000
#define BAT_PERCENTAGE_UPDATE_INTERVAL_US 1 * 60 * 1000 * 1000
#define MAX_BAT_VOLTAGE 4.2
#define MIN_BAT_VOLTAGE 3.0
#define MAX_BAR_RAW_VALUE 4096

// #define SHOW_BAT_VOLTAGE
#define SLEEP_MODE_ON

static const char *TAG = "SCREENS";

static animation_frame beating_heart[HEART_ANIM_FRAMES_QTY] = {
    {.x = 105, .y = 65, .icon = heart_icon, .color = RED_COLOR, .scale = 1},
    {.x = 90, .y = 50, .icon = heart_icon, .color = RED_COLOR, .scale = 2},
};

enum Screen screen_number = STARTUP;
static enum Screen last_screen_number = STARTUP;

const uint8_t MAX_SCREENS_QTY = TOTAL_COUNT - 1;

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

void startup_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *startup = "STARTING";
        gfx_draw_text(60, 20, startup, SEA_GREEN_COLOR, 2);
    }
}

static void draw_bat_percentage(enum Screen_Event event) {
    static uint8_t displayed_percentage;
    static uint64_t last_update = 0;

    static bool charging_state_displayed = false;

    bool is_charging = gpio_get_level(PIN_CHARGE) == 1;
    int filtered = 0;
    adc_read_filtered(PIN_BAT, &filtered);
    float voltage = (float)filtered * (MAX_BAT_VOLTAGE / MAX_BAR_RAW_VALUE);
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

static void screen_light_sleep(uint64_t sleep_time) {
#ifdef SLEEP_MODE_ON
    esp_sleep_enable_timer_wakeup(TIME_SCREEN_SLEEP_TIMEOUT_US);
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

static void time_screen(enum Screen_Event event, imu_sensor *imu) {
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

static void weather_screen(enum Screen_Event event) {
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

static void heart_screen(enum Screen_Event event, hr_sensor *hr) {
    if (event == ENTER) {
        display_clear();

        const char *screen_name = "Heart rate";
        gfx_draw_text(35, 0, screen_name, WHITE_COLOR, 2);

        gfx_draw_icon(90, 50, heart_icon, RED_COLOR, 2);

        const char *heart_rate = "--";
        gfx_draw_text(85, 140, heart_rate, WHITE_COLOR, 5);
    }

    static bool is_measuring = false;
    static uint32_t bpm = 0;
    static uint32_t last_bpm = 0;

    hr_read_bpm(hr, &is_measuring, &bpm);

    if (is_measuring) {
        gfx_animation(90, 50, 65, 60, beating_heart, HEART_ANIM_FRAMES_QTY, HEART_DRAW_DELAY_US);

        if (bpm != 0 && last_bpm != bpm) {
            char bpm_buffer[5];
            snprintf(bpm_buffer, sizeof(bpm_buffer), "%ld", bpm);
            gfx_fill_rect(85, 140, 90, 35, BLACK_COLOR); // clear bpm
            gfx_draw_text(85, 140, bpm_buffer, WHITE_COLOR, 5);
        }
    } else {
        bpm = 0;

        if (last_bpm != bpm) {
            gfx_draw_icon(90, 50, heart_icon, RED_COLOR, 2);
            gfx_fill_rect(85, 140, 90, 35, BLACK_COLOR); // clear bpm

            const char *heart_rate = "--";
            gfx_draw_text(85, 140, heart_rate, WHITE_COLOR, 5);
        }
    }

    last_bpm = bpm;
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