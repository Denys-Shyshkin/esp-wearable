#include "time.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include <esp_log.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define MAX_RETRY_COUNT 15

static const char *TAG = "TIME";

uint8_t retry_count = 0;

bool sync_time(void) {
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);

    while (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)) != ESP_OK && ++retry_count < MAX_RETRY_COUNT) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d/%d)", retry_count, MAX_RETRY_COUNT);
    }

    if (retry_count == MAX_RETRY_COUNT) {
        ESP_LOGE(TAG, "Failed to sync time after %d attempts", retry_count);
        return false;
    }

    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", strftime_buf);

    esp_netif_sntp_deinit();
    return true;
}