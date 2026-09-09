#pragma once

#include "drivers/hr/hr.h"
#include "drivers/imu/imu.h"
#include "drivers/button/button.h"
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
void screen_change(button *btn_up, button *btn_down);
void screen_manager(imu_sensor *imu, hr_sensor *hr);
void screen_light_sleep(uint64_t sleep_time);