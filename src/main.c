#include "display/display.h"
#include "display/graphics.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SUPERLOOP_DELAY 100

// static const char *TAG = "MAIN";

void app_main() {
    display_init();

    const char *text = "Weather forecast";
    
    const char *hours = "20";
    const char *separator = ":";
    const char *mins = "56";

    gfx_draw_text(0, 20, text, 0xFFFF, 2);

    gfx_draw_text(0, 50, hours, 0xFFFF, 7);
    gfx_draw_text(102, 55, separator, 0xFFFF, 5);
    gfx_draw_text(130, 50, mins, 0xFFFF, 7);

    while (1) {

        vTaskDelay(pdMS_TO_TICKS(SUPERLOOP_DELAY));
    }
}