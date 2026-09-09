#include "config/config.h"
#include "driver/i2c_master.h"
#include "drivers/adc/adc.h"
#include "drivers/button/button.h"
#include "drivers/display/display.h"
#include "drivers/hr/hr.h"
#include "drivers/i2c/i2c.h"
#include "drivers/imu/imu.h"
#include "drivers/wifi/wifi.h"
#include "esp_sleep.h"
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

#define INIT_STATUSES_QTY 2

#define SUPERLOOP_DELAY 10

// static const char *TAG = "MAIN";

typedef struct {
    const char *text;
    uint16_t color;
} init_status;

init_status const fail_status = {.text = "FAIL", .color = RED_COLOR};
init_status const ok_status = {.text = "OK", .color = GREEN_COLOR};

init_status init_statuses[INIT_STATUSES_QTY] = {fail_status, ok_status};

i2c_master_bus_handle_t i2c_bus_0;
imu_sensor imu;
hr_sensor hr;

button btn_up = DEFAULT_BUTTON;
button btn_down = DEFAULT_BUTTON;

static void main_functionality_setup() {
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

    const char *hr_sensor = "HR sensor.......";
    gfx_draw_text(40, 160, hr_sensor, LIGHT_GREY_COLOR, 1);
    uint8_t is_hr_sensor_init = hr_sensor_init(&i2c_bus_0, &hr);
    gfx_draw_text(170, 160, init_statuses[is_hr_sensor_init].text, init_statuses[is_hr_sensor_init].color, 1);
}

static void bat_charge_gpio_init() {
    gpio_config_t io_config = {
        .pin_bit_mask = 1ULL << PIN_CHARGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_config));
}

static void sleep_functionality_setup() {
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(PIN_BUTTON_UP, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(PIN_BUTTON_DOWN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup_on_hp_periph_powerdown(1ULL << PIN_BUTTON_UP, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_sleep_enable_gpio_wakeup_on_hp_periph_powerdown(1ULL << PIN_BUTTON_DOWN, ESP_GPIO_WAKEUP_GPIO_LOW);
}

void app_main() {
    display_init();
    i2c_bus_init(&i2c_bus_0);
    adc_init();
    bat_charge_gpio_init();

    main_functionality_setup();

    sleep_functionality_setup();

    buttons_init(&btn_up, &btn_down);

    while (1) {
        weather_update();

        buttons_reading(&btn_up, &btn_down);
        screen_change(&btn_up, &btn_down);
        screen_manager(&imu, &hr);

        // esp_deep_sleep_start();

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY));
    }
}