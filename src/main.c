#include "driver/i2c_master.h"
#include "drivers/button/button.h"
#include "drivers/display/display.h"
#include "drivers/i2c/i2c.h"
#include "drivers/imu/imu.h"
#include "drivers/wifi/wifi.h"
#include "esp_timer.h"
#include "services/graphics/font_8x8.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"
#include "services/http_requests/http_get.h"
#include "services/parser/weather.h"
#include "services/screens/screens.h"
#include "services/time/time.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define BUTTON_UP GPIO_NUM_7
#define BUTTON_DOWN GPIO_NUM_8

#define INIT_STATUSES_QTY 2
#define WEATHER_UPDATE_DELAY_US 30 * 60 * 1000 * 1000 // 30 mins

#define SUPERLOOP_DELAY 10

static const char *TAG = "MAIN";

#define DEFAULT_BUTTON (Button){.gpio = -1, .btn_state = 1, .last_btn_state = 1, .s_last_btn_pressed = 0, .is_btn_pressed = 0}

Button btn_up = DEFAULT_BUTTON;
Button btn_down = DEFAULT_BUTTON;

typedef struct {
    const char *text;
    uint16_t color;
} Init_Status;

Init_Status const fail_status = {.text = "FAIL", .color = RED_COLOR};
Init_Status const ok_status = {.text = "OK", .color = GREEN_COLOR};

Init_Status init_statuses[INIT_STATUSES_QTY] = {fail_status, ok_status};

i2c_master_bus_handle_t i2c_bus_0;
imu_sensor imu;
imu_raw_data imu_raw;

static void buttons_reading() {
    button_is_pressed(&btn_up);
    button_is_pressed(&btn_down);
}

static void screen_change() {
    if (btn_up.is_btn_pressed) {
        btn_up.is_btn_pressed = 0;

        if (screen_number >= MAX_SCREENS_QTY) {
            screen_number = 1;
        } else {
            screen_number++;
        }
    }

    if (btn_down.is_btn_pressed) {
        btn_down.is_btn_pressed = 0;

        if (screen_number <= 1) {
            screen_number = MAX_SCREENS_QTY;
        } else {
            screen_number--;
        }
    }
}

static bool weather_update() {
    uint32_t now = esp_timer_get_time();
    static uint32_t last_weather_update = 0;

    if (now - last_weather_update >= WEATHER_UPDATE_DELAY_US || last_weather_update == 0) {
        last_weather_update = now;

        uint8_t is_request_succeed = http_get(weather_url);
        if (is_request_succeed) {
            return parse_weather();
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

static bool pedometer_init(i2c_master_bus_handle_t *bus, imu_sensor *imu) {
    esp_err_t imu_init_error = imu_init(&i2c_bus_0, imu);

    if (imu_init_error != ESP_OK) {
        ESP_LOGI(TAG, "IMU initialization failed: %s", esp_err_to_name(imu_init_error));
        return false;
    }

    esp_err_t imu_pedometer_error = imu_pedometer_config(imu);

    if (imu_pedometer_error != ESP_OK) {
        ESP_LOGI(TAG, "IMU pedometer config failed: %s", esp_err_to_name(imu_pedometer_error));
        return false;
    }

    return true;
}

static void functionality_setup() {
    startup_screen(ENTER);

    const char *wifi_connection = "Wi-Fi...........";
    gfx_draw_text(40, 80, wifi_connection, LIGHT_GREY_COLOR, 1);
    uint8_t is_wifi_init = wifi_init_sta();
    gfx_draw_text(170, 80, init_statuses[is_wifi_init].text, init_statuses[is_wifi_init].color, 1);

    const char *time_synced = "Time synced.....";
    gfx_draw_text(40, 100, time_synced, LIGHT_GREY_COLOR, 1);
    uint8_t is_sync_time = sync_time();
    gfx_draw_text(170, 100, init_statuses[is_sync_time].text, init_statuses[is_sync_time].color, 1);

    const char *weather_updated = "Weather update..";
    gfx_draw_text(40, 120, weather_updated, LIGHT_GREY_COLOR, 1);
    uint8_t is_weather_updated = weather_update();
    gfx_draw_text(170, 120, init_statuses[is_weather_updated].text, init_statuses[is_weather_updated].color, 1);

    const char *mui_init = "IMU init........";
    gfx_draw_text(40, 140, mui_init, LIGHT_GREY_COLOR, 1);
    uint8_t is_pedometer_inited = pedometer_init(&i2c_bus_0, &imu);
    gfx_draw_text(170, 140, init_statuses[is_pedometer_inited].text, init_statuses[is_pedometer_inited].color, 1);
}

void app_main() {
    display_init();
    i2c_bus_init(&i2c_bus_0);

    functionality_setup();

    button_init(&btn_up, BUTTON_UP);
    button_init(&btn_down, BUTTON_DOWN);

    while (1) {
        weather_update();

        buttons_reading();
        screen_change();
        screen_manager(&imu);

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY));
    }
}