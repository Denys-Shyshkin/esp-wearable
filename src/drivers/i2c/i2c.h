#pragma once

#include "driver/i2c_master.h"

void i2c_scanner(i2c_master_bus_handle_t *bus);
void i2c_bus_init(i2c_master_bus_handle_t *bus);
esp_err_t i2c_read_regs(i2c_master_dev_handle_t i2c_dev, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_write_single_reg(i2c_master_dev_handle_t i2c_dev, uint8_t reg, uint8_t value);