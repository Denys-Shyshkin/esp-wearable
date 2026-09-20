#include "config/config.h"
#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"

#define HEART_DRAW_DELAY_US 400 * 1000
#define HEART_ANIM_FRAMES_QTY 2

static animation_frame beating_heart[HEART_ANIM_FRAMES_QTY] = {
    {.x = 105, .y = 65, .icon = heart_icon, .color = COLOR_ERROR, .scale = 1},
    {.x = 90, .y = 50, .icon = heart_icon, .color = COLOR_ERROR, .scale = 2},
};

void heart_screen(enum Screen_Event event, hr_sensor *hr, bool func_status[]) {
    if (event == ENTER) {
        display_clear();

        const char *screen_name = "HEART RATE";
        gfx_draw_text(40, 0, screen_name, COLOR_SECONDARY, 2);

        gfx_draw_icon(90, 50, heart_icon, func_status[HR_SENSOR] ? COLOR_ERROR : COLOR_PRIMARY, 2);

        if (func_status[HR_SENSOR]) {
            const char *heart_rate = "--";
            gfx_draw_text(85, 140, heart_rate, COLOR_PRIMARY, 5);
        } else {
            const char *not_found = "Sensor not found";
            gfx_draw_text(60, 140, not_found, COLOR_ERROR, 1);
        }
    }

    if (func_status[HR_SENSOR]) {
        static bool is_measuring = false;
        static uint32_t bpm = 0;
        static uint32_t last_bpm = 0;

        hr_read_bpm(hr, &is_measuring, &bpm);

        if (is_measuring) {
            gfx_animation(90, 50, 65, 60, beating_heart, HEART_ANIM_FRAMES_QTY, HEART_DRAW_DELAY_US);

            if (bpm != 0 && last_bpm != bpm) {
                char bpm_buffer[5];
                snprintf(bpm_buffer, sizeof(bpm_buffer), "%ld", bpm);
                gfx_fill_rect(85, 140, 90, 35, COLOR_BACKGROUND); // clear bpm
                gfx_draw_text(85, 140, bpm_buffer, COLOR_PRIMARY, 5);
            }
        } else {
            bpm = 0;

            if (last_bpm != bpm) {
                gfx_draw_icon(90, 50, heart_icon, COLOR_ERROR, 2);
                gfx_fill_rect(85, 140, 90, 35, COLOR_BACKGROUND); // clear bpm

                const char *heart_rate = "--";
                gfx_draw_text(85, 140, heart_rate, COLOR_PRIMARY, 5);
            }
        }

        last_bpm = bpm;
    }
}