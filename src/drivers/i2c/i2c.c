#include "i2c.h"
#include "driver/i2c_master.h"
#include <esp_log.h>

#define PIN_SDA GPIO_NUM_1
#define PIN_SCL GPIO_NUM_0

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