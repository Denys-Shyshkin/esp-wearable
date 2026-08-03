#pragma once

#include <stdint.h>

void gfx_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void gfx_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void gfx_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void gfx_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale);
void gfx_draw_text(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t scale);