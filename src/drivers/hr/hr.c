#include "hr.h"
#include "driver/i2c_master.h"
#include "drivers/i2c/i2c.h"
#include "esp_timer.h"
#include <esp_log.h>

#define I2C_ADDR 0x57
#define PART_ID_ADDR 0xFF
#define REV_ID_ADDR 0xFE
#define FIFO_WR_PTR_ADDR 0x04
#define FIFO_OVF_COUNTER_ADDR 0x05
#define FIFO_RD_PTR_ADDR 0x06
#define FIFO_DATA_ADDR 0x07

// FIFO_ROLLOVER_EN = 1
#define FIFO_CONFIG_VALUE 0x10
#define FIFO_CONFIG_ADDR 0x08

// SPO2_ADC_RGE  = 00  - 2048 nA full scale ADC range
// SPO2_SR       = 001 - 100 samples per sec
// LED_PW        = 11  - 411 µs pulse width with 18-bit ADC resolution
#define SPO2_CONFIG_VALUE 0x07
#define SPO2_CONFIG_ADDR 0x0A // config used for SpO2 and heart-rate modes

#define MODE_CONFIG_HR_ONLY 0x02
#define MODE_CONFIG_RESET 0x40
#define MODE_CONFIG_ADDR 0x09

// 0x10 = 16 × 0.2 mA = 3.2 mA
#define PULSE_AMPLITUDE_1_VALUE 0x10
#define PULSE_AMPLITUDE_1_ADDR 0x0C

#define HR_ALPHA 0.02f // smoothing factor
#define BEAT_THRESHOLD_UPPER 50
#define BEAT_THRESHOLD_LOWER -50
#define DETECTION_THRESHOLD 9500
#define BMP_AVERAGE_TIME 5 * 1000 * 1000
#define SIGNAL_CALMING_DELAY 1 * 1000 * 1000
#define SMA_WINDOW_SIZE 8
#define INTERVAL_WINDOW_SIZE 20

static const char *TAG = "HR";

static uint32_t hr_combine_fifo_data(uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    return ((uint32_t)(byte0 & 0x03) << 16) | ((uint32_t)byte1 << 8) | (uint32_t)byte2;
}

static esp_err_t hr_setup(i2c_master_bus_handle_t *bus, hr_sensor *hr) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus, &dev_cfg, &hr->i2c_dev));

    uint8_t part_id;
    esp_err_t part_id_error = i2c_read_regs(hr->i2c_dev, PART_ID_ADDR, &part_id, 1);
    ESP_LOGI(TAG, "Part ID: 0x%02X", part_id);
    if (part_id_error != ESP_OK) {
        return part_id_error;
    }

    uint8_t rev_id;
    esp_err_t rev_id_error = i2c_read_regs(hr->i2c_dev, REV_ID_ADDR, &rev_id, 1);
    ESP_LOGI(TAG, "Revision ID: 0x%02X", rev_id);
    if (rev_id_error != ESP_OK) {
        return rev_id_error;
    }

    return ESP_OK;
}

static esp_err_t hr_reset(hr_sensor *hr) {
    esp_err_t reset_error = i2c_write_single_reg(hr->i2c_dev, MODE_CONFIG_ADDR, MODE_CONFIG_RESET);
    if (reset_error != ESP_OK) {
        return reset_error;
    }

    return ESP_OK;
}

static esp_err_t hr_controls_config(hr_sensor *hr) {
    return i2c_write_single_reg(hr->i2c_dev, SPO2_CONFIG_ADDR, SPO2_CONFIG_VALUE);
}

static esp_err_t hr_mode_config(hr_sensor *hr) {
    uint8_t mode_config;
    esp_err_t mode_read_error = i2c_read_regs(hr->i2c_dev, MODE_CONFIG_ADDR, &mode_config, 1);
    if (mode_read_error != ESP_OK) {
        return mode_read_error;
    }

    mode_config |= MODE_CONFIG_HR_ONLY;
    esp_err_t mode_write_error = i2c_write_single_reg(hr->i2c_dev, MODE_CONFIG_ADDR, mode_config);
    if (mode_write_error != ESP_OK) {
        return mode_write_error;
    }

    return ESP_OK;
}

static esp_err_t hr_pulse_amplitude(hr_sensor *hr) {
    uint8_t pulse_amplitude;
    esp_err_t pa_read_error = i2c_read_regs(hr->i2c_dev, PULSE_AMPLITUDE_1_ADDR, &pulse_amplitude, 1);
    if (pa_read_error != ESP_OK) {
        return pa_read_error;
    }

    pulse_amplitude |= PULSE_AMPLITUDE_1_VALUE;
    esp_err_t pa_write_error = i2c_write_single_reg(hr->i2c_dev, PULSE_AMPLITUDE_1_ADDR, pulse_amplitude);
    if (pa_write_error != ESP_OK) {
        return pa_write_error;
    }

    return ESP_OK;
}

static esp_err_t hr_fifo_config(hr_sensor *hr) {
    uint8_t fifo_config;
    esp_err_t fifo_config_read_error = i2c_read_regs(hr->i2c_dev, FIFO_CONFIG_ADDR, &fifo_config, 1);
    if (fifo_config_read_error != ESP_OK) {
        return fifo_config_read_error;
    }

    fifo_config |= FIFO_CONFIG_VALUE;
    esp_err_t fifo_config_write_error = i2c_write_single_reg(hr->i2c_dev, FIFO_CONFIG_ADDR, fifo_config);
    if (fifo_config_write_error != ESP_OK) {
        return fifo_config_write_error;
    }

    return ESP_OK;
}

esp_err_t hr_init(i2c_master_bus_handle_t *bus, hr_sensor *hr) {
    esp_err_t setup_error = hr_setup(bus, hr);
    if (setup_error != ESP_OK) {
        return setup_error;
    }

    esp_err_t reset_error = hr_reset(hr);
    if (reset_error != ESP_OK) {
        return reset_error;
    }

    esp_err_t controls_config_error = hr_controls_config(hr);
    if (controls_config_error != ESP_OK) {
        return controls_config_error;
    }

    esp_err_t mode_config_error = hr_mode_config(hr);
    if (mode_config_error != ESP_OK) {
        return mode_config_error;
    }

    esp_err_t amplitude_error = hr_pulse_amplitude(hr);
    if (amplitude_error != ESP_OK) {
        return amplitude_error;
    }

    esp_err_t fifo_error = hr_fifo_config(hr);
    if (fifo_error != ESP_OK) {
        return fifo_error;
    }

    return ESP_OK;
}

esp_err_t hr_read_raw(hr_sensor *hr, bool *is_available, uint32_t *raw) {
    *is_available = false;

    uint8_t wr_ptr;
    esp_err_t wr_ptr_error = i2c_read_regs(hr->i2c_dev, FIFO_WR_PTR_ADDR, &wr_ptr, 1);
    if (wr_ptr_error != ESP_OK) {
        return wr_ptr_error;
    }

    uint8_t rd_ptr;
    esp_err_t rd_ptr_error = i2c_read_regs(hr->i2c_dev, FIFO_RD_PTR_ADDR, &rd_ptr, 1);
    if (rd_ptr_error != ESP_OK) {
        return rd_ptr_error;
    }

    if (wr_ptr == rd_ptr) {
        return ESP_OK;
    }

    uint8_t fifo_data[3];
    esp_err_t data_error = i2c_read_regs(hr->i2c_dev, FIFO_DATA_ADDR, fifo_data, 3);
    if (data_error != ESP_OK) {
        return data_error;
    }

    *raw = hr_combine_fifo_data(fifo_data[0], fifo_data[1], fifo_data[2]);
    *is_available = true;

    return ESP_OK;
}

static uint32_t hr_smooth_data(uint32_t *raw) {
    static uint32_t sample_buffer[SMA_WINDOW_SIZE];
    static uint8_t buffer_index = 0;
    static uint8_t samples_filled = 0;
    static uint32_t running_sum = 0;

    uint32_t sample = *raw;

    running_sum -= sample_buffer[buffer_index];
    sample_buffer[buffer_index] = sample;
    running_sum += sample;

    buffer_index = (buffer_index + 1) % SMA_WINDOW_SIZE;

    if (samples_filled < SMA_WINDOW_SIZE) {
        samples_filled++;
    }

    return running_sum / samples_filled;
}

static int64_t hr_filter_data(uint32_t *raw) {
    static float dc_estimate;
    static bool is_first_read = true;

    float sample = (float)*raw;

    if (is_first_read) {
        is_first_read = false;
        dc_estimate = sample;
    }

    dc_estimate = dc_estimate + HR_ALPHA * (sample - dc_estimate);

    return sample - dc_estimate;
}

static uint8_t hr_interval_to_bpm(uint32_t *interval_time) {
    return 60 * 1000 * 1000 / *interval_time;
}

esp_err_t hr_read_bpm(hr_sensor *hr) {
    static uint32_t touch_detected_time = 0;
    static bool rising_state = false;
    static uint32_t last_beat_time = 0;
    static uint32_t last_bpm_record = 0;
    static bool buffer_full = false;
    static uint32_t interval_buffer[INTERVAL_WINDOW_SIZE] = {0};
    static uint8_t buffer_index = 0;

    uint32_t now = esp_timer_get_time();
    bool is_available;
    uint32_t raw;

    esp_err_t raw_error = hr_read_raw(hr, &is_available, &raw);
    if (raw_error != ESP_OK) {
        return raw_error;
    }

    if (is_available) {
        if (raw > DETECTION_THRESHOLD) {
            if (touch_detected_time == 0) {
                touch_detected_time = now;
            }
            uint32_t smooth = hr_smooth_data(&raw);
            int64_t filtered = hr_filter_data(&smooth);

            if (now - touch_detected_time >= SIGNAL_CALMING_DELAY) {
                if (rising_state && filtered > BEAT_THRESHOLD_UPPER) {
                    rising_state = false;

                    if (last_beat_time != 0) {
                        interval_buffer[buffer_index] = now - last_beat_time;

                        buffer_index++;

                        if (buffer_index >= INTERVAL_WINDOW_SIZE) {
                            buffer_index = 0;
                            buffer_full = true;
                        }
                    }

                    last_beat_time = now;
                } else if (!rising_state && filtered < BEAT_THRESHOLD_LOWER) {
                    rising_state = true;
                }

                if (now - last_bpm_record >= BMP_AVERAGE_TIME) {
                    uint8_t intervals_qty = buffer_full ? INTERVAL_WINDOW_SIZE : (buffer_index > 1 ? buffer_index - 1 : 0);

                    if (intervals_qty > 0) {
                        uint32_t interval_sums = 0;

                        for (int i = 0; i < intervals_qty; i++) {
                            interval_sums += interval_buffer[i];
                        }

                        uint32_t avg_interval_time = interval_sums / intervals_qty;
                        uint32_t bpm = hr_interval_to_bpm(&avg_interval_time);

                        ESP_LOGI(TAG, "BPM: %ld | intervals: %d", bpm, intervals_qty);
                    }

                    last_bpm_record = now;
                }
            }
        } else {
            touch_detected_time = 0;
            buffer_index = 0;
            last_beat_time = 0;
            buffer_full = false;

            for (int i = 0; i < INTERVAL_WINDOW_SIZE; i++) {
                interval_buffer[i] = 0;
            }
        }
    }

    return ESP_OK;
}