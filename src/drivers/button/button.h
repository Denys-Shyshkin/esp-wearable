#pragma once

#include "driver/gpio.h"

#define DEFAULT_BUTTON (button){.gpio = -1, .btn_state = 1, .last_btn_state = 1, .s_last_btn_pressed = 0, .is_btn_pressed = 0}

typedef struct {
    gpio_num_t gpio;
    bool btn_state;
    bool last_btn_state;
    uint64_t s_last_btn_pressed;
    bool is_btn_pressed;
} button;

void buttons_init(button *btn_up, button *btn_down);
void buttons_reading(button *btn_up, button *btn_down);