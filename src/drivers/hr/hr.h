#pragma once

#include "driver/i2c_master.h"

typedef struct {
    i2c_master_dev_handle_t i2c_dev;
} hr_sensor;

esp_err_t hr_init(i2c_master_bus_handle_t *bus, hr_sensor *hr);
esp_err_t hr_read_raw(hr_sensor *hr, bool *is_available, uint32_t *raw);
esp_err_t hr_read_bpm(hr_sensor *hr);