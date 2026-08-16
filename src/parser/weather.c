#include "weather.h"
#include "cJSON.h"
#include "http_requests/http_get.h"
#include <esp_log.h>

static const char *TAG = "WEATHER PARSER";

const char *weather_url = "http://jaam.net.ua/weather_statuses_v2.json"; 

void parse_weather() {
    response_buffer[response_len] = '\0';

    cJSON *root = cJSON_Parse(response_buffer);

    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return;
    }

    cJSON *states = cJSON_GetObjectItem(root, "states");

    if (states == NULL) {
        ESP_LOGE(TAG, "No 'states' object");
        cJSON_Delete(root);
        return;
    }

    cJSON *kyiv = cJSON_GetObjectItem(states, "14");

    if (kyiv == NULL) {
        ESP_LOGE(TAG, "No region 14");
        cJSON_Delete(root);
        return;
    }

    cJSON *temp = cJSON_GetObjectItem(kyiv, "temp");
    cJSON *feels_like = cJSON_GetObjectItem(kyiv, "feels_like");
    cJSON *humidity = cJSON_GetObjectItem(kyiv, "humidity");
    cJSON *pressure = cJSON_GetObjectItem(kyiv, "pressure");

    Weather_Data weather = {0};

    if (cJSON_IsNumber(temp)) {
        weather.temperature = temp->valuedouble;
    }

    if (cJSON_IsNumber(feels_like)) {
        weather.feels_like = feels_like->valuedouble;
    }

    if (cJSON_IsNumber(humidity)) {
        weather.humidity = humidity->valueint;
    }

    if (cJSON_IsNumber(pressure)) {
        weather.pressure = pressure->valueint;
    }

    ESP_LOGI(TAG, "Temperature: %.2f °C", weather.temperature);
    ESP_LOGI(TAG, "Feels like: %.2f °C", weather.feels_like);
    ESP_LOGI(TAG, "Humidity: %d %%", weather.humidity);
    ESP_LOGI(TAG, "Pressure: %d hPa", weather.pressure);

    cJSON_Delete(root);
}