#pragma once

#include <stdint.h>

void display_init();
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void display_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
bool display_validate_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void display_clip_rect(uint16_t x, uint16_t y, uint16_t *width, uint16_t *height);
void display_clear();