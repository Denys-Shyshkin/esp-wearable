#include "config/config.h"
#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/font_8x8.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"
#include "services/parser/weather.h"

#define WEATHER_SCREEN_SLEEP_TIMEOUT_US 15 * 60 * 1000 * 1000

static void draw_weather_icon(enum Screen_Event event) {
    static char *displayed_icon;

    if (displayed_icon != weather.icon || event == ENTER) {
        gfx_fill_rect(10, 45, 60, 60, COLOR_BACKGROUND); // clear icon
        const uint32_t *icon = get_weather_icon(weather.icon);

        if (icon != NULL) {
            gfx_draw_icon(10, 45, icon, COLOR_PRIMARY, 2);
        }

        displayed_icon = weather.icon;
    }
}

static void draw_temperature(enum Screen_Event event) {
    static uint8_t displayed_temp;

    if (displayed_temp != weather.temperature || event == ENTER) {
        gfx_fill_rect(80, 60, 80, 50, COLOR_BACKGROUND); // clear temp
        char temp_buffer[5];
        snprintf(temp_buffer, sizeof(temp_buffer), "%d", weather.temperature);
        gfx_draw_text(80, 60, temp_buffer, COLOR_PRIMARY, 5);

        displayed_temp = weather.temperature;
    }
}

static void draw_wind(enum Screen_Event event) {
    static float displayed_wind;

    if (displayed_wind != weather.wind_speed || event == ENTER) {
        gfx_fill_rect(80, 140, 80, 20, COLOR_BACKGROUND); // clear wind
        char wind_buffer[10];
        snprintf(wind_buffer, sizeof(wind_buffer), "%.1f", weather.wind_speed);
        gfx_draw_text(80, 140, wind_buffer, COLOR_PRIMARY, 2);

        displayed_wind = weather.wind_speed;
    }
}

static void draw_pressure(enum Screen_Event event) {
    static uint16_t displayed_pressure;

    if (displayed_pressure != weather.pressure || event == ENTER) {
        gfx_fill_rect(80, 180, 80, 20, COLOR_BACKGROUND); // clear pressure
        char pressure_buffer[10];
        snprintf(pressure_buffer, sizeof(pressure_buffer), "%d", weather.pressure);
        gfx_draw_text(80, 180, pressure_buffer, COLOR_PRIMARY, 2);

        displayed_pressure = weather.pressure;
    }
}

static void draw_humidity(enum Screen_Event event) {
    static uint8_t displayed_humidity;

    if (displayed_humidity != weather.humidity || event == ENTER) {
        gfx_fill_rect(80, 220, 80, 20, COLOR_BACKGROUND); // clear humidity
        char humidity_buffer[10];
        snprintf(humidity_buffer, sizeof(humidity_buffer), "%d", weather.humidity);
        gfx_draw_text(80, 220, humidity_buffer, COLOR_PRIMARY, 2);

        displayed_humidity = weather.humidity;
    }
}

void weather_screen(enum Screen_Event event, bool func_status[]) {
    if (event == ENTER) {
        display_clear();

        const char *location_name = "KYIV";
        gfx_draw_text(90, 0, location_name, COLOR_SECONDARY, 2);

        gfx_draw_line(60, 30, 180, 30, COLOR_PRIMARY);

        if (func_status[WEATHER_UPDATE]) {
            gfx_draw_spec_char(160, 60, degree_char, COLOR_PRIMARY, 4);

            const char *temp_unit = "C";
            gfx_draw_text(190, 60, temp_unit, COLOR_PRIMARY, 5);

            gfx_draw_line(20, 120, 220, 120, COLOR_PRIMARY);

            gfx_draw_icon(30, 130, wind_icon, COLOR_PRIMARY, 1);
            const char *wind_units = "km/h";
            gfx_draw_text(160, 140, wind_units, COLOR_PRIMARY, 2);

            gfx_draw_icon(30, 170, pressure_icon, COLOR_PRIMARY, 1);
            const char *pressure_units = "mbar";
            gfx_draw_text(160, 180, pressure_units, COLOR_PRIMARY, 2);

            gfx_draw_icon(30, 210, humidity_icon, COLOR_PRIMARY, 1);

            const char *humidity_units = "%";
            gfx_draw_text(160, 220, humidity_units, COLOR_PRIMARY, 2);
        } else {
            const char *no_data = "No data";
            gfx_draw_text(70, 50, no_data, COLOR_ERROR, 2);
        }
    }

    if (func_status[WEATHER_UPDATE]) {
        draw_weather_icon(event);
        draw_temperature(event);
        draw_wind(event);
        draw_pressure(event);
        draw_humidity(event);
    }

#ifdef MCU_LIGHT_SLEEP_MODE_ON
    screen_light_sleep(WEATHER_SCREEN_SLEEP_TIMEOUT_US);
#endif
}