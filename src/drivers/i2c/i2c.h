#pragma once

#include "driver/i2c_master.h"

void i2c_scanner(i2c_master_bus_handle_t *bus);
void i2c_bus_init(i2c_master_bus_handle_t *bus);