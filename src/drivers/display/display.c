#include "display.h"
#include "config/config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "drivers/button/button.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_timer.h"
#include <esp_log.h>
#include <math.h>

#define LCD_H_RES 240
#define LCD_V_RES 240
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define BKL_PWM_FREQ_HZ 5000
#define BKL_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define BKL_PWM_MAX_DUTY 1023

#define LCD_HOST SPI2_HOST

static const char *TAG = "DISPLAY";

static esp_lcd_panel_handle_t panel_handle = NULL;
bool is_display_sleep = false;
bool is_display_inactive = false;

static bool display_validate_start(uint16_t x, uint16_t y) {
    if (x >= LCD_H_RES || y >= LCD_V_RES) {
        return false;
    }

    return true;
}

bool display_validate_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (width == 0 || height == 0) {
        return false;
    }

    if (!display_validate_start(x, y)) {
        return false;
    }

    return true;
}

void display_clip_rect(uint16_t x, uint16_t y, uint16_t *width, uint16_t *height) {
    // Clip if beyond the edges
    if (x + *width > LCD_H_RES) {
        *width = LCD_H_RES - x;
    }
    if (y + *height > LCD_V_RES) {
        *height = LCD_V_RES - y;
    }
}

void display_brightness_percentage(uint8_t brightness) {
    if (brightness > 100) {
        brightness = 100;
    }

    // Convert 0-100% perceptual brightness into a gamma-corrected PWM duty cycle.
    float normalized = brightness / 100.0f;
    float corrected = powf(normalized, 2.2f);

    uint32_t duty = (uint32_t)(corrected * BKL_PWM_MAX_DUTY);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void display_init() {
    ESP_LOGI(TAG, "Initialize LCD backlight PWM");

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = BKL_PWM_RESOLUTION,
        .freq_hz = BKL_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = PIN_BKL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = BKL_PWM_MAX_DUTY,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_SCLK,
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t), // transfer 80 lines of pixels (assume pixel is RGB565) at most in one SPI transaction
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Allocate an LCD IO device handle from the SPI bus");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_DC,
        .cs_gpio_num = PIN_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install the LCD controller driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    display_brightness_percentage(100);

    display_clear();
}

void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!display_validate_start(x, y)) {
        return;
    }

    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 1, y + 1, &color);
}

void display_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (!display_validate_rect(x, y, width, height)) {
        return;
    }

    display_clip_rect(x, y, &width, &height);

    // Filling buffer for one row
    static uint16_t buffer[LCD_H_RES];
    for (uint16_t i = 0; i < width; i++) {
        buffer[i] = color;
    }

    // Iteration thru all rows
    for (uint16_t row = y; row < (y + height); row++) {
        esp_lcd_panel_draw_bitmap(panel_handle, x, row, x + width, row + 1, buffer);
    }
}

void display_clear() {
    display_fill_rect(0, 0, 240, 240, 0x0000);
}

void display_auto_inactive(uint64_t inactive_timeout) {
    uint64_t now = esp_timer_get_time();

    if (now - last_button_interaction >= inactive_timeout && !is_display_inactive) {
        display_brightness_percentage(50);

        is_display_inactive = true;
    }
}

void display_auto_sleep(uint64_t sleep_timeout) {
    uint64_t now = esp_timer_get_time();

    if (now - last_button_interaction >= sleep_timeout && !is_display_sleep) {
        esp_lcd_panel_disp_sleep(panel_handle, true);
        display_brightness_percentage(0);

        is_display_sleep = true;
    }
}

void display_wakeup() {
    esp_lcd_panel_disp_sleep(panel_handle, false);
    display_brightness_percentage(100);

    is_display_inactive = false;
}