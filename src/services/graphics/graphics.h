#pragma once

#include <stdint.h>

#define WHITE_COLOR 0xFFFF
#define BLACK_COLOR 0x0000
#define RED_COLOR 0xF800
#define GREEN_COLOR 0x07E0
#define BLUE_COLOR 0x001F
#define YELLOW_COLOR 0xFFE0

#define LIGHT_GREY_COLOR 0x8410
#define LIGHT_BLUE_COLOR 0x8E7F
#define SEA_GREEN_COLOR 0x4574

typedef struct {
    int16_t x;
    int16_t y;
    const uint32_t *icon;
    uint16_t color;
    uint8_t scale;
} animation_frame;

void gfx_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void gfx_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void gfx_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_base_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale);
void gfx_draw_text(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t scale);
void gfx_draw_spec_char(int16_t x, int16_t y, const uint8_t special_char[], uint16_t color, uint8_t scale);
void gfx_draw_icon(int16_t x, int16_t y, const uint32_t *icon, uint16_t color, uint8_t scale);
void gfx_draw_alignment_lines();
void gfx_animation(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const animation_frame frames[], uint8_t frames_qty, uint32_t frames_delay);