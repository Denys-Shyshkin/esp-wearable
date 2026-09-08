#include "adc.h"
#include "config/config.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "utils/utils.h"
#include <esp_log.h>

#define SMA_WINDOW_SIZE 50

adc_oneshot_unit_handle_t adc_handle;

void adc_init() {
    adc_oneshot_unit_init_cfg_t init_cfg = {.unit_id = ADC_UNIT_1};

    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, PIN_BAT, &chan_cfg));
}

void adc_read_raw(adc_channel_t chan, int *raw) {
    adc_oneshot_read(adc_handle, chan, raw);
}

void adc_read_filtered(adc_channel_t chan, int *filtered) {
    int raw;
    adc_read_raw(chan, &raw);
    *filtered = utils_moving_average(&raw, SMA_WINDOW_SIZE);
}