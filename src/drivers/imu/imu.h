#pragma once

#include "driver/i2c_master.h"

typedef struct {
    i2c_master_dev_handle_t i2c_dev;
} imu_sensor;

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} imu_raw_data;

bool imu_init(i2c_master_bus_handle_t *bus, imu_sensor *imu);
esp_err_t imu_read_raw(imu_sensor *imu, imu_raw_data *raw);