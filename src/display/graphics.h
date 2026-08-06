#pragma once

#include <stdint.h>

#define WHITE_COLOR 0xFFFF
#define BLACK_COLOR 0x0000
#define RED_COLOR 0xF800
#define GREEN_COLOR 0x07E0
#define BLUE_COLOR 0x001F
#define YELLOW_COLOR 0xFFE0

void gfx_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void gfx_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void gfx_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_base_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale);
void gfx_draw_text(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t scale);
void gfx_draw_spec_char(int16_t x, int16_t y, const uint8_t special_char[], uint16_t color, uint8_t scale);
void gfx_draw_alignment_lines();