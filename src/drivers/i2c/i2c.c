#include "i2c.h"
#include "driver/i2c_master.h"
#include <esp_log.h>

#define PIN_SDA GPIO_NUM_1
#define PIN_SCL GPIO_NUM_0

#define I2C_TIMEOUT_MS 100

static const char *TAG = "I2C";

void i2c_scanner(i2c_master_bus_handle_t *bus) {
    int found = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        if (i2c_master_probe(*bus, a, 50) == ESP_OK) {
            ESP_LOGI(TAG, "  ACK -> 0x%02X (8-bit: W=0x%02X R=0x%02X)", a, a << 1, (a << 1) | 1);
            found++;
        }
    }
    ESP_LOGI(TAG, "Total found: %d", found);
}

void i2c_bus_init(i2c_master_bus_handle_t *bus) {
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, bus));
}

esp_err_t i2c_read_regs(i2c_master_dev_handle_t i2c_dev, uint8_t reg, uint8_t *data, size_t len) {
    if (i2c_dev == NULL || data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(i2c_dev, &reg, 1, data, len, I2C_TIMEOUT_MS);
}

esp_err_t i2c_write_single_reg(i2c_master_dev_handle_t i2c_dev, uint8_t reg, uint8_t value) {
    if (i2c_dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[2];

    data[0] = reg;
    data[1] = value;

    return i2c_master_transmit(i2c_dev, data, sizeof(data), I2C_TIMEOUT_MS);
}