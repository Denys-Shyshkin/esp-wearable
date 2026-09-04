#pragma once

#include "drivers/imu/imu.h"
#include "drivers/hr/hr.h"
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
extern const uint8_t MAX_SCREENS_QTY;

void go_screen_up();
void go_screen_down();
void startup_screen(enum Screen_Event event);
void screen_manager(imu_sensor *imu, hr_sensor *hr);