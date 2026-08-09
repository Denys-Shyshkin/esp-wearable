#pragma once

#include <stdint.h>

enum Screen_Event {
    ENTER,
    UPDATE,
};

enum Screen {
    STARTUP,
    TIME,
    WEATHER,
    HEART,
    TOTAL_COUNT,
};

extern enum Screen screen_number;
extern enum Screen last_screen_number;
extern const uint8_t MAX_SCREENS_QTY;

void startup_screen(enum Screen_Event event);
void time_screen(enum Screen_Event event);
void weather_screen(enum Screen_Event event);
void heart_screen(enum Screen_Event event);
void screen_manager();