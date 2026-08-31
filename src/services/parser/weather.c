#include "weather.h"
#include "cJSON.h"
#include "services/http_requests/http_get.h"
#include "services/time/time.h"
#include <esp_log.h>
#include <time.h>

static const char *TAG = "WEATHER PARSER";

const char *weather_url = "http://jaam.net.ua/weather_statuses_v2.json";

weather_data weather = {0};

bool parse_weather() {
    response_buffer[response_len] = '\0';

    cJSON *root = cJSON_Parse(response_buffer);

    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return 0;
    }

    cJSON *states = cJSON_GetObjectItem(root, "states");

    if (states == NULL) {
        ESP_LOGE(TAG, "No 'states' object");
        cJSON_Delete(root);
        return 0;
    }

    cJSON *kyiv = cJSON_GetObjectItem(states, "14");

    if (kyiv == NULL) {
        ESP_LOGE(TAG, "No region 14");
        cJSON_Delete(root);
        return 0;
    }

    cJSON *temp = cJSON_GetObjectItem(kyiv, "temp");
    cJSON *wind_speed = cJSON_GetObjectItem(kyiv, "wind_speed");
    cJSON *pressure = cJSON_GetObjectItem(kyiv, "pressure");
    cJSON *humidity = cJSON_GetObjectItem(kyiv, "humidity");
    cJSON *weather_array = cJSON_GetObjectItem(kyiv, "weather");

    if (cJSON_IsNumber(temp)) {
        weather.temperature = temp->valueint;
    }

    if (cJSON_IsNumber(wind_speed)) {
        weather.wind_speed = wind_speed->valuedouble;
    }

    if (cJSON_IsNumber(pressure)) {
        weather.pressure = pressure->valueint;
    }

    if (cJSON_IsNumber(humidity)) {
        weather.humidity = humidity->valueint;
    }

    if (cJSON_IsArray(weather_array)) {
        cJSON *weather_array_item = cJSON_GetArrayItem(weather_array, 0);

        if (weather_array_item != NULL) {
            cJSON *icon = cJSON_GetObjectItem(weather_array_item, "icon");

            if (cJSON_IsString(icon)) {
                strncpy(weather.icon, icon->valuestring, sizeof(weather.icon) - 1);
                weather.icon[sizeof(weather.icon) - 1] = '\0';
            }
        }
    }

    struct tm timeinfo;
    get_time(&timeinfo);

    char time_buff[6];
    strftime(time_buff, sizeof(time_buff), "%H:%M", &timeinfo);

    ESP_LOGI(TAG, "Weather last parsing: %s", time_buff);

    cJSON_Delete(root);

    return 1;
}