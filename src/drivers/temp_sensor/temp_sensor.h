#pragma once

#include "driver/i2c_master.h"
#include <stdint.h>

typedef struct {
    i2c_master_dev_handle_t i2c_dev;

    uint32_t raw;

    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    int32_t t_fine;
} temp_sensor;

void temp_sensor_init(i2c_master_bus_handle_t *bus, temp_sensor *sensor);
void temp_sensor_read_temperature(temp_sensor *sensor, int32_t *temperature);