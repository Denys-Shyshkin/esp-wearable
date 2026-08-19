#include "temp_sensor.h"
#include "driver/i2c_master.h"
#include <esp_log.h>

void temp_sensor_init(i2c_master_bus_handle_t *bus, temp_sensor *sensor) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x76,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &dev_cfg, &sensor->i2c_dev));
}

static esp_err_t temp_sensor_read_regs(temp_sensor *sensor, uint8_t reg, uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_master_transmit_receive(sensor->i2c_dev, &reg, 1, data, len, -1);
}

static esp_err_t temp_sensor_read_raw(temp_sensor *sensor) {
    uint8_t data[3];
    esp_err_t err = temp_sensor_read_regs(sensor, 0xFA, data, 3);

    if (err != ESP_OK) {
        return err;
    }

    sensor->raw = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);

    return ESP_OK;
}

static esp_err_t temp_sensor_read_calibration(temp_sensor *sensor) {
    uint8_t data[6];
    esp_err_t err = temp_sensor_read_regs(sensor, 0x88, data, 6);

    if (err != ESP_OK) {
        return err;
    }

    sensor->dig_T1 = ((uint16_t)data[1] << 8) | data[0];
    sensor->dig_T2 = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    sensor->dig_T3 = (int16_t)(((uint16_t)data[5] << 8) | data[4]);

    return ESP_OK;
}

static void temp_sensor_calibrate_raw_temperature(temp_sensor *sensor) {
    int32_t var1 = ((((sensor->raw >> 3) - ((int32_t)sensor->dig_T1 << 1))) * (int32_t)sensor->dig_T2) >> 11;
    int32_t var2 = (((((sensor->raw >> 4) - (int32_t)sensor->dig_T1) * ((sensor->raw >> 4) - (int32_t)sensor->dig_T1)) >> 12) * (int32_t)sensor->dig_T3) >> 14;

    sensor->t_fine = var1 + var2;
}

void temp_sensor_read_temperature(temp_sensor *sensor, int32_t *temperature) {
    temp_sensor_read_raw(sensor);
    temp_sensor_read_calibration(sensor);
    temp_sensor_calibrate_raw_temperature(sensor);

    *temperature = (sensor->t_fine * 5 + 128) >> 8;
}