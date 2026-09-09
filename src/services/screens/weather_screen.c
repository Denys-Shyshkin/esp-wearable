#include "screen_manager.h"
#include "drivers/display/display.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"
#include "services/graphics/font_8x8.h"
#include "services/parser/weather.h"

static void draw_weather_icon(enum Screen_Event event) {
    static char *displayed_icon;

    if (displayed_icon != weather.icon || event == ENTER) {
        gfx_fill_rect(10, 45, 60, 60, BLACK_COLOR); // clear icon
        const uint32_t *icon = get_weather_icon(weather.icon);

        if (icon != NULL) {
            gfx_draw_icon(10, 45, icon, WHITE_COLOR, 2);
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