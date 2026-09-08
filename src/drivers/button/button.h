#pragma once

#include "driver/gpio.h"

typedef struct {
    gpio_num_t gpio;
    bool btn_state;
    bool last_btn_state;
    uint64_t s_last_btn_pressed;
    bool is_btn_pressed;
} button;

void button_init(button *btn, gpio_num_t gpio);
void button_is_pressed(button *btn);