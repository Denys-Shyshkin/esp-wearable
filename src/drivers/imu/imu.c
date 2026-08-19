#include "imu.h"
#include "driver/i2c_master.h"
#include <esp_log.h>

#define QMI8658_I2C_ADDR 0x6B
#define QMI8658_ID_ADDR 0x00
#define QMI8658_OUTPUT_STATUS_ADDR 0x2E
#define QMI8658_AX_L_ADDR 0x35

#define QMI8658_ID 0x05
#define I2C_TIMEOUT_MS 100

// SIM            = 0 - no SPI but I²C
// ADDR_AI        = 1 - enable address auto-increment
// BE             = 0 - little-endian
// INT2_EN        = 0 - no interrupt
// INT1_EN        = 0 - no interrupt
// FIFO_INT_SEL   = 0 - no FIFO
// SensorDisable  = 0 - sensor oscillator enabled
#define QMI8658_CTRL1_VALUE 0x40
#define QMI8658_CTRL1_ADDR 0x02

// aST  = 0 - Disable Accelerometer Self-Test;
// aFS  = 001 - Accelerometer Full-scale = ±4 g
// aODR = 0110 - 125 Hz ODR in Normal mode
#define QMI8658_CTRL2_VALUE 0x16
#define QMI8658_CTRL2_ADDR 0x03

// gST  = 0 - Disable Gyro self-Test
// gFS  = 101 - ±512 dps
// gODR = 0110 - 112.1 Hz ODR in Normal mode
#define QMI8658_CTRL3_VALUE 0x56
#define QMI8658_CTRL3_ADDR 0x04

// SyncSample = 0 - normal/non-sync mode
// DRDY_DIS   = 0 - keep data-ready enabled
// gSN        = 0 - gyro full mode
// gEN        = 1 - enable gyro
// aEN        = 1 - enable accelerometer
#define QMI8658_CTRL7_VALUE 0x03
#define QMI8658_CTRL7_ADDR 0x08

static const char *TAG = "IMU";

// TODO: generic function - move to i2c driver
static esp_err_t imu_read_regs(imu_sensor *imu, uint8_t reg, uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(imu->i2c_dev, &reg, 1, data, len, I2C_TIMEOUT_MS);
}

static esp_err_t imu_write_single_reg(imu_sensor *imu, uint8_t reg, uint8_t value) {
    if (imu == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[2];

    data[0] = reg;
    data[1] = value;

    return i2c_master_transmit(imu->i2c_dev, data, sizeof(data), I2C_TIMEOUT_MS);
}

static void imu_config_ctrl1(imu_sensor *imu) {
    imu_write_single_reg(imu, QMI8658_CTRL1_ADDR, QMI8658_CTRL1_VALUE);

    uint8_t ctrl1;
    esp_err_t err = imu_read_regs(imu, QMI8658_CTRL1_ADDR, &ctrl1, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL1 = 0x%02X", ctrl1);
    } else {
        ESP_LOGI(TAG, "CTRL1 config failed: %s", esp_err_to_name(err));
    }
}

static void imu_config_ctrl2(imu_sensor *imu) {
    imu_write_single_reg(imu, QMI8658_CTRL2_ADDR, QMI8658_CTRL2_VALUE);

    uint8_t ctrl2;
    esp_err_t err = imu_read_regs(imu, QMI8658_CTRL2_ADDR, &ctrl2, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL2 = 0x%02X", ctrl2);
    } else {
        ESP_LOGI(TAG, "CTRL2 config failed: %s", esp_err_to_name(err));
    }
}

static void imu_config_ctrl3(imu_sensor *imu) {
    imu_write_single_reg(imu, QMI8658_CTRL3_ADDR, QMI8658_CTRL3_VALUE);

    uint8_t ctrl3;
    esp_err_t err = imu_read_regs(imu, QMI8658_CTRL3_ADDR, &ctrl3, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL3 = 0x%02X", ctrl3);
    } else {
        ESP_LOGI(TAG, "CTRL3 config failed: %s", esp_err_to_name(err));
    }
}

static void imu_config_ctrl7(imu_sensor *imu) {
    imu_write_single_reg(imu, QMI8658_CTRL7_ADDR, QMI8658_CTRL7_VALUE);

    uint8_t ctrl7;
    esp_err_t err = imu_read_regs(imu, QMI8658_CTRL7_ADDR, &ctrl7, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL7 = 0x%02X", ctrl7);
    } else {
        ESP_LOGI(TAG, "CTRL7 config failed: %s", esp_err_to_name(err));
    }
}

// TODO: use array of struct
static void imu_config(imu_sensor *imu) {
    imu_config_ctrl1(imu);
    imu_config_ctrl2(imu);
    imu_config_ctrl3(imu);
    imu_config_ctrl7(imu);
}

bool imu_init(i2c_master_bus_handle_t *bus, imu_sensor *imu) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = QMI8658_I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &dev_cfg, &imu->i2c_dev));

    uint8_t imu_id;
    esp_err_t err = imu_read_regs(imu, QMI8658_ID_ADDR, &imu_id, 1);

    ESP_LOGI(TAG, "Who am I: 0x%02X", imu_id);
    if (imu_id == QMI8658_ID) {
        imu_config(imu);
        return true;
    } else {
        ESP_LOGI(TAG, "Initialization failed: %s", esp_err_to_name(err));
        return false;
    }
}

static int16_t imu_combine_int16(uint8_t low, uint8_t high) {
    return (high << 8) | low;
}

esp_err_t imu_read_raw(imu_sensor *imu, imu_raw_data *raw) {
    if (imu == NULL || raw == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t status;
    esp_err_t status_err = imu_read_regs(imu, QMI8658_OUTPUT_STATUS_ADDR, &status, 1);

    if (status_err != ESP_OK) {
        return status_err;
    }

    // 00000011 - both accelerometer and gyroscope data available
    if ((status & 0x03) == 0x03) {
        uint8_t raw_buffer[12];
        esp_err_t data_err = imu_read_regs(imu, QMI8658_AX_L_ADDR, raw_buffer, 12);

        if (data_err != ESP_OK) {
            return data_err;
        }

        raw->accel_x = imu_combine_int16(raw_buffer[0], raw_buffer[1]);
        raw->accel_y = imu_combine_int16(raw_buffer[2], raw_buffer[3]);
        raw->accel_z = imu_combine_int16(raw_buffer[4], raw_buffer[5]);

        raw->gyro_x = imu_combine_int16(raw_buffer[6], raw_buffer[7]);
        raw->gyro_y = imu_combine_int16(raw_buffer[8], raw_buffer[9]);
        raw->gyro_z = imu_combine_int16(raw_buffer[10], raw_buffer[11]);

        return ESP_OK;
    }

    return ESP_ERR_NOT_FINISHED;
}
