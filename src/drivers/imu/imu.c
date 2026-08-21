#include "imu.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include <esp_log.h>

#define I2C_ADDR 0x6B
#define ID_ADDR 0x00
#define OUTPUT_STATUSINT_ADDR 0x2D
#define OUTPUT_STATUS0_ADDR 0x2E
#define AX_L_ADDR 0x35
#define STEP_CNT_LOW_ADDR 0x5A
#define CAL1_L_ADDR 0x0B

#define QMI8658_ID 0x05
#define I2C_TIMEOUT_MS 100
#define CMD_DONE_TIMEOUT_US 1000 * 1000

// SIM            = 0 - no SPI but I²C
// ADDR_AI        = 1 - enable address auto-increment
// BE             = 0 - little-endian
// INT2_EN        = 0 - no interrupt
// INT1_EN        = 0 - no interrupt
// FIFO_INT_SEL   = 0 - no FIFO
// SensorDisable  = 0 - sensor oscillator enabled
#define CTRL1_VALUE 0x40
#define CTRL1_ADDR 0x02

// aST  = 0 - Disable Accelerometer Self-Test;
// aFS  = 001 - Accelerometer Full-scale = ±4 g
// aODR = 0110 - 125 Hz ODR in Normal mode
#define CTRL2_VALUE 0x16
#define CTRL2_ADDR 0x03

// gST  = 0 - Disable Gyro self-Test
// gFS  = 101 - ±512 dps
// gODR = 0110 - 112.1 Hz ODR in Normal mode
#define CTRL3_VALUE 0x56
#define CTRL3_ADDR 0x04

// SyncSample = 0 - normal/non-sync mode
// DRDY_DIS   = 0 - keep data-ready enabled
// gSN        = 0 - gyro full mode
// gEN        = 1 - enable gyro
// aEN        = 1 - enable accelerometer
#define CTRL7_ENABLE_VALUE 0x03
#define CTRL7_DISABLE_VALUE 0x00
#define CTRL7_ADDR 0x08

#define CTRL8_ADDR 0x09

#define CTRL9_ACK 0x00
#define CTRL9_CONFIGURE_PEDOMETER_VALUE 0x0D
#define CTRL9_ADDR 0x0A

// Pedometer first group config
#define CAL1_L_1 0x32 // ped_sample_cnt = 50
#define CAL1_H_1 0x00
#define CAL2_L_1 0xCC // ped_fix_peak2peak ≈ 200 mg
#define CAL2_H_1 0x00
#define CAL3_L_1 0x66 // ped_fix_peak ≈ 100 mg
#define CAL3_H_1 0x00
#define CAL4_L_1 0x00 // NA
#define CAL4_H_1 0x01 // parameters group 1

// Pedometer second group config
#define CAL1_L_2 0xF4 // ped_time_up = 500
#define CAL1_H_2 0x01
#define CAL2_L_2 0x32 // ped_time_low = 50
#define CAL2_H_2 0x0A // ped_time_cnt_entry = 10
#define CAL3_L_2 0x00 // ped_fix_precision = 0
#define CAL3_H_2 0x04 // ped_sig_count = 4
#define CAL4_L_2 0x00 // NA
#define CAL4_H_2 0x02 // parameters group 2

#define PEDOMETER_CONFIG_LENGTH 8

static const char *TAG = "IMU";

const uint8_t config_group_1[PEDOMETER_CONFIG_LENGTH] = {CAL1_L_1, CAL1_H_1, CAL2_L_1, CAL2_H_1, CAL3_L_1, CAL3_H_1, CAL4_L_1, CAL4_H_1};
const uint8_t config_group_2[PEDOMETER_CONFIG_LENGTH] = {CAL1_L_2, CAL1_H_2, CAL2_L_2, CAL2_H_2, CAL3_L_2, CAL3_H_2, CAL4_L_2, CAL4_H_2};

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
    imu_write_single_reg(imu, CTRL1_ADDR, CTRL1_VALUE);

    uint8_t ctrl1;
    esp_err_t err = imu_read_regs(imu, CTRL1_ADDR, &ctrl1, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL1 = 0x%02X", ctrl1);
    } else {
        ESP_LOGI(TAG, "CTRL1 config failed: %s", esp_err_to_name(err));
    }
}

static void imu_config_ctrl2(imu_sensor *imu) {
    imu_write_single_reg(imu, CTRL2_ADDR, CTRL2_VALUE);

    uint8_t ctrl2;
    esp_err_t err = imu_read_regs(imu, CTRL2_ADDR, &ctrl2, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL2 = 0x%02X", ctrl2);
    } else {
        ESP_LOGI(TAG, "CTRL2 config failed: %s", esp_err_to_name(err));
    }
}

static void imu_config_ctrl3(imu_sensor *imu) {
    imu_write_single_reg(imu, CTRL3_ADDR, CTRL3_VALUE);

    uint8_t ctrl3;
    esp_err_t err = imu_read_regs(imu, CTRL3_ADDR, &ctrl3, 1);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CTRL3 = 0x%02X", ctrl3);
    } else {
        ESP_LOGI(TAG, "CTRL3 config failed: %s", esp_err_to_name(err));
    }
}

static esp_err_t imu_enable(imu_sensor *imu) {
    return imu_write_single_reg(imu, CTRL7_ADDR, CTRL7_ENABLE_VALUE);
}

// TODO: use array of struct
static void imu_config(imu_sensor *imu) {
    imu_config_ctrl1(imu);
    imu_config_ctrl2(imu);
    imu_config_ctrl3(imu);
    imu_enable(imu);
}

esp_err_t imu_init(i2c_master_bus_handle_t *bus, imu_sensor *imu) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &dev_cfg, &imu->i2c_dev));

    uint8_t imu_id;
    esp_err_t err = imu_read_regs(imu, ID_ADDR, &imu_id, 1);

    ESP_LOGI(TAG, "Who am I: 0x%02X", imu_id);
    if (imu_id == QMI8658_ID) {
        imu_config(imu);
        return ESP_OK;
    } else {
        ESP_LOGI(TAG, "Initialization failed: %s", esp_err_to_name(err));
        return err;
    }
}

static int16_t imu_combine_2_bytes(uint8_t low, uint8_t high) {
    return (high << 8) | low;
}

static uint32_t imu_combine_3_bytes(uint8_t low, uint8_t mid, uint8_t high) {
    return (high << 16) | (mid << 8) | low;
}

esp_err_t imu_read_raw(imu_sensor *imu, imu_raw_data *raw) {
    if (imu == NULL || raw == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t status;
    esp_err_t status_err = imu_read_regs(imu, OUTPUT_STATUS0_ADDR, &status, 1);

    if (status_err != ESP_OK) {
        return status_err;
    }

    // 00000011 - both accelerometer and gyroscope data available
    if ((status & 0x03) == 0x03) {
        uint8_t raw_buffer[12];
        esp_err_t data_err = imu_read_regs(imu, AX_L_ADDR, raw_buffer, 12);

        if (data_err != ESP_OK) {
            return data_err;
        }

        raw->accel_x = imu_combine_2_bytes(raw_buffer[0], raw_buffer[1]);
        raw->accel_y = imu_combine_2_bytes(raw_buffer[2], raw_buffer[3]);
        raw->accel_z = imu_combine_2_bytes(raw_buffer[4], raw_buffer[5]);

        raw->gyro_x = imu_combine_2_bytes(raw_buffer[6], raw_buffer[7]);
        raw->gyro_y = imu_combine_2_bytes(raw_buffer[8], raw_buffer[9]);
        raw->gyro_z = imu_combine_2_bytes(raw_buffer[10], raw_buffer[11]);

        return ESP_OK;
    }

    return ESP_ERR_NOT_FINISHED;
}

static esp_err_t imu_disable(imu_sensor *imu) {
    return imu_write_single_reg(imu, CTRL7_ADDR, CTRL7_DISABLE_VALUE);
}

static esp_err_t imu_check_cmd_done(imu_sensor *imu) {
    uint8_t status = 0;
    uint64_t start_time = esp_timer_get_time();

    while ((status & (1 << 7)) == 0) {
        uint64_t now = esp_timer_get_time();

        esp_err_t status_error = imu_read_regs(imu, OUTPUT_STATUSINT_ADDR, &status, 1);
        if (status_error != ESP_OK) {
            return status_error;
        }

        if (now - start_time > CMD_DONE_TIMEOUT_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    return ESP_OK;
}

static esp_err_t imu_apply_config(imu_sensor *imu) {
    return imu_write_single_reg(imu, CTRL9_ADDR, CTRL9_CONFIGURE_PEDOMETER_VALUE);
}

static esp_err_t imu_ack_command(imu_sensor *imu) {
    return imu_write_single_reg(imu, CTRL9_ADDR, CTRL9_ACK);
}

static esp_err_t imu_pedometer_apply_config(imu_sensor *imu, const uint8_t *config_set, uint8_t config_length) {
    if (imu == NULL || config_set == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t start_addr = CAL1_L_ADDR;
    for (int i = 0; i < config_length; i++) {
        esp_err_t write_error = imu_write_single_reg(imu, start_addr + i, config_set[i]);

        if (write_error != ESP_OK) {
            return write_error;
        }
    }

    esp_err_t apply_error = imu_apply_config(imu);
    if (apply_error != ESP_OK) {
        return apply_error;
    }

    esp_err_t check_error = imu_check_cmd_done(imu);
    if (check_error != ESP_OK) {
        return check_error;
    }

    esp_err_t ack_error = imu_ack_command(imu);
    if (ack_error != ESP_OK) {
        return ack_error;
    }

    return ESP_OK;
}

static esp_err_t imu_pedometer_enable(imu_sensor *imu) {
    uint8_t ctrl8;
    esp_err_t read_reg = imu_read_regs(imu, CTRL8_ADDR, &ctrl8, 1);
    if (read_reg != ESP_OK) {
        return read_reg;
    }

    ctrl8 |= (1 << 4);
    esp_err_t write_error = imu_write_single_reg(imu, CTRL8_ADDR, ctrl8);
    if (write_error != ESP_OK) {
        return write_error;
    }

    return ESP_OK;
}

esp_err_t imu_pedometer_config(imu_sensor *imu) {
    esp_err_t disable_error = imu_disable(imu);
    if (disable_error != ESP_OK) {
        return disable_error;
    }

    esp_err_t config_1_error = imu_pedometer_apply_config(imu, config_group_1, PEDOMETER_CONFIG_LENGTH);
    if (config_1_error != ESP_OK) {
        return config_1_error;
    }

    esp_err_t config_2_error = imu_pedometer_apply_config(imu, config_group_2, PEDOMETER_CONFIG_LENGTH);
    if (config_2_error != ESP_OK) {
        return config_2_error;
    }

    esp_err_t enable_pedometer_error = imu_pedometer_enable(imu);
    if (enable_pedometer_error != ESP_OK) {
        return enable_pedometer_error;
    }

    esp_err_t enable_imu_error = imu_enable(imu);
    if (enable_imu_error != ESP_OK) {
        return enable_imu_error;
    }

    return ESP_OK;
}

esp_err_t imu_read_steps(imu_sensor *imu, uint32_t *steps) {
    if (imu == NULL || steps == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t steps_buffer[3];
    esp_err_t read_err = imu_read_regs(imu, STEP_CNT_LOW_ADDR, steps_buffer, 3);

    if (read_err != ESP_OK) {
        return read_err;
    }

    *steps = imu_combine_3_bytes(steps_buffer[0], steps_buffer[1], steps_buffer[2]);

    return ESP_OK;
}