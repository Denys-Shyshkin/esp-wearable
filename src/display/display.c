#include "display.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_vendor.h"
#include <esp_log.h>

#define PIN_NUM_SCLK 4
#define PIN_NUM_MOSI 6
#define PIN_NUM_MISO 3
#define PIN_NUM_DC 10
#define PIN_NUM_CS 20
#define PIN_NUM_BK_LIGHT 21
#define PIN_NUM_LCD_RST 9

#define LCD_H_RES 240
#define LCD_V_RES 240
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_HOST SPI2_HOST

static const char *TAG = "DISPLAY";

static esp_lcd_panel_handle_t panel_handle = NULL;

// ========================== HELPERS ==========================

bool display_validate_start(uint16_t x, uint16_t y) {
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

// ========================== INIT ==========================

void display_init() {
    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_BK_LIGHT, 1));

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t), // transfer 80 lines of pixels (assume pixel is RGB565) at most in one SPI transaction
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Allocate an LCD IO device handle from the SPI bus");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install the LCD controller driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
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

    display_clear();
}

// ========================== DRAW ==========================

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
    for (int i = 0; i < width; i++) {
        buffer[i] = color;
    }

    // Iteration thru all rows
    for (int row = y; row < (y + height); row++) {
        esp_lcd_panel_draw_bitmap(panel_handle, x, row, x + width, row + 1, buffer);
    }
}

void display_clear() {
    display_fill_rect(0, 0, 240, 240, 0x0000);
}