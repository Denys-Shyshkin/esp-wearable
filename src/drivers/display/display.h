#pragma once

#include <stdint.h>

extern bool is_display_inactive;
extern bool is_display_sleep;

void display_init();
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void display_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
bool display_validate_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void display_clip_rect(uint16_t x, uint16_t y, uint16_t *width, uint16_t *height);
void display_clear();
void display_auto_inactive(uint64_t inactive_timeout);
void display_auto_sleep(uint64_t sleep_timeout);
void display_wakeup();