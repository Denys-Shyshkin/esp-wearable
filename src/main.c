#include "config/config.h"
#include "driver/i2c_master.h"
#include "drivers/adc/adc.h"
#include "drivers/button/button.h"
#include "drivers/display/display.h"
#include "drivers/hr/hr.h"
#include "drivers/i2c/i2c.h"
#include "drivers/imu/imu.h"
#include "drivers/wifi/wifi.h"
#include "drivers/battery/battery.h"
#include "esp_sleep.h"
#include "services/graphics/graphics.h"
#include "services/parser/weather.h"
#include "services/screens/screen_manager.h"
#include "services/time/time.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "services/screens/startup_screen.h"

#define INIT_STATUSES_QTY 2
#define SUPERLOOP_DELAY 10

#define DISPLAY_INACTIVE_TIMEOUT 1 * 60 * 1000 * 1000

// static const char *TAG = "MAIN";

typedef struct {
    const char *text;
    uint16_t color;
} init_status;

init_status const fail_status = {.text = "FAIL", .color = RED_COLOR};
init_status const ok_status = {.text = "OK", .color = GREEN_COLOR};

init_status init_statuses[INIT_STATUSES_QTY] = {fail_status, ok_status};

bool func_status[FUNC_TOTAL_COUNT] = {false};

i2c_master_bus_handle_t i2c_bus_0;
imu_sensor imu;
hr_sensor hr;

button btn_up = DEFAULT_BUTTON;
button btn_down = DEFAULT_BUTTON;

static void main_functionality_setup() {
    startup_screen(ENTER, func_status);

    const char *wifi_connection = "Wi-Fi...........";
    gfx_draw_text(40, 80, wifi_connection, LIGHT_GREY_COLOR, 1);
    func_status[WIFI] = wifi_init_sta();
    gfx_draw_text(170, 80, init_statuses[func_status[WIFI]].text, init_statuses[func_status[WIFI]].color, 1);

    const char *time_synced = "Time synced.....";
    gfx_draw_text(40, 100, time_synced, LIGHT_GREY_COLOR, 1);
    func_status[TIME_SYNC] = sync_time();
    gfx_draw_text(170, 100, init_statuses[func_status[TIME_SYNC]].text, init_statuses[func_status[TIME_SYNC]].color, 1);

    const char *weather_updated = "Weather update..";
    gfx_draw_text(40, 120, weather_updated, LIGHT_GREY_COLOR, 1);
    func_status[WEATHER_UPDATE] = weather_update();
    gfx_draw_text(170, 120, init_statuses[func_status[WEATHER_UPDATE]].text, init_statuses[func_status[WEATHER_UPDATE]].color, 1);

    const char *mui_init = "IMU init........";
    gfx_draw_text(40, 140, mui_init, LIGHT_GREY_COLOR, 1);
    func_status[IMU_SENSOR] = pedometer_init(&i2c_bus_0, &imu);
    gfx_draw_text(170, 140, init_statuses[func_status[IMU_SENSOR]].text, init_statuses[func_status[IMU_SENSOR]].color, 1);

    const char *hr_sensor = "HR sensor.......";
    gfx_draw_text(40, 160, hr_sensor, LIGHT_GREY_COLOR, 1);
    func_status[HR_SENSOR] = hr_sensor_init(&i2c_bus_0, &hr);
    gfx_draw_text(170, 160, init_statuses[func_status[HR_SENSOR]].text, init_statuses[func_status[HR_SENSOR]].color, 1);
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
    
    main_functionality_setup();
    
    sleep_functionality_setup();
    
    buttons_init(&btn_up, &btn_down);
    battery_charge_pin_init();
    
    while (1) {
        weather_update();

        buttons_reading(&btn_up, &btn_down);
        screen_change(&btn_up, &btn_down, func_status);
        screen_manager(&imu, &hr, func_status);

        // esp_deep_sleep_start();
        display_auto_inactive(DISPLAY_INACTIVE_TIMEOUT);

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY));
    }
}