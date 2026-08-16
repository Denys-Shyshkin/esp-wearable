#include "graphics.h"
#include "drivers/display/display.h"
#include "esp_timer.h"
#include "font_8x8.h"
#include "icons.h"
#include <esp_log.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// static const char *TAG = "GFX";

static uint16_t max(uint16_t a, uint16_t b) {
    return (a > b) ? a : b;
}

void gfx_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    display_draw_pixel(x, y, color);
}

void gfx_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = x1 - x0;
    int16_t dy = y1 - y0;

    uint16_t steps = max(abs(dx), abs(dy));

    if (steps == 0) {
        gfx_draw_pixel(x0, y0, color);
        return;
    }

    float x_increment = (float)dx / steps;
    float y_increment = (float)dy / steps;

    float x = x0;
    float y = y0;

    for (uint16_t i = 0; i <= steps; i++) {
        gfx_draw_pixel(roundf(x), roundf(y), color);

        x += x_increment;
        y += y_increment;
    }
}

void gfx_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    display_fill_rect(x, y, width, height, color);
}

void gfx_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (!display_validate_rect(x, y, width, height)) {
        return;
    }

    display_clip_rect(x, y, &width, &height);

    uint16_t x1 = x + width - 1;
    uint16_t y1 = y + height - 1;

    gfx_draw_line(x, y, x1, y, color);
    gfx_draw_line(x1, y, x1, y1, color);
    gfx_draw_line(x1, y1, x, y1, color);
    gfx_draw_line(x, y1, x, y, color);
}

static void gfx_draw_char(int16_t x, int16_t y, const uint8_t *glyph, uint16_t color, uint8_t scale) {
    for (uint8_t row = 0; row < FONT_8X8_ROWS_QTY; row++) {
        uint8_t row_data = glyph[row];

        for (uint8_t col = 0; col < FONT_8X8_COLS_QTY; col++) {
            bool pixel_on = (row_data >> col) & 1;

            if (pixel_on) {
                gfx_fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void gfx_draw_icon(int16_t x, int16_t y, const uint32_t *icon, uint16_t color, uint8_t scale) {
    for (uint8_t row = 0; row < ICON_ROWS_QTY; row++) {
        uint32_t row_data = icon[row];

        for (uint8_t col = 0; col < ICON_COLS_QTY; col++) {
            bool pixel_on = (row_data >> col) & 1;

            if (pixel_on) {
                gfx_fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void gfx_draw_base_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale) {
    if (scale == 0) {
        return;
    }
    if (c < FONT_8X8_FIRST_CHAR || c > FONT_8X8_LAST_CHAR) {
        return;
    }

    uint8_t index = c - FONT_8X8_FIRST_CHAR;

    gfx_draw_char(x, y, font_8x8[index], color, scale);
}

void gfx_draw_text(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t scale) {
    if (scale == 0) {
        return;
    }
    int16_t cursor_x = x;

    while (*text != '\0') {
        gfx_draw_base_char(cursor_x, y, *text, color, scale);

        text++;
        cursor_x += (FONT_8X8_COLS_QTY + FONT_8X8_SPACING) * scale;
    }
}

void gfx_draw_spec_char(int16_t x, int16_t y, const uint8_t special_char[], uint16_t color, uint8_t scale) {
    if (scale == 0) {
        return;
    }

    gfx_draw_char(x, y, special_char, color, scale);
}

void gfx_draw_alignment_lines() {
    // Screen center lines
    gfx_draw_line(120, 0, 120, 240, RED_COLOR);
    gfx_draw_line(0, 120, 240, 120, RED_COLOR);

    // Screen 1/3 grid
    gfx_draw_line(0, 80, 240, 80, YELLOW_COLOR);
    gfx_draw_line(0, 160, 240, 160, YELLOW_COLOR);
    gfx_draw_line(80, 0, 80, 240, YELLOW_COLOR);
    gfx_draw_line(160, 0, 160, 240, YELLOW_COLOR);
}

void gfx_animation(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const Animation_Frame frames[], uint8_t frames_qty, uint32_t frames_delay) {
    uint32_t now = esp_timer_get_time();

    static uint8_t current_frame = 0;
    static uint32_t last_frame_draw = 0;

    if (now - last_frame_draw >= frames_delay) {
        last_frame_draw = now;

        gfx_fill_rect(x, y, width, height, BLACK_COLOR);
        gfx_draw_icon(frames[current_frame].x, frames[current_frame].y, frames[current_frame].icon, frames[current_frame].color, frames[current_frame].scale);

        current_frame++;
        if (current_frame >= frames_qty) {
            current_frame = 0;
        }
    }
}