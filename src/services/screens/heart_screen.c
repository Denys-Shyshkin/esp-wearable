#include "screen_manager.h"
#include "drivers/display/display.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"

#define HEART_DRAW_DELAY_US 400 * 1000
#define HEART_ANIM_FRAMES_QTY 2

static animation_frame beating_heart[HEART_ANIM_FRAMES_QTY] = {
    {.x = 105, .y = 65, .icon = heart_icon, .color = RED_COLOR, .scale = 1},
    {.x = 90, .y = 50, .icon = heart_icon, .color = RED_COLOR, .scale = 2},
};

void heart_screen(enum Screen_Event event, hr_sensor *hr) {
    if (event == ENTER) {
        display_clear();

        const char *screen_name = "Heart rate";
        gfx_draw_text(35, 0, screen_name, WHITE_COLOR, 2);

        gfx_draw_icon(90, 50, heart_icon, RED_COLOR, 2);

        const char *heart_rate = "--";
        gfx_draw_text(85, 140, heart_rate, WHITE_COLOR, 5);
    }

    static bool is_measuring = false;
    static uint32_t bpm = 0;
    static uint32_t last_bpm = 0;

    hr_read_bpm(hr, &is_measuring, &bpm);

    if (is_measuring) {
        gfx_animation(90, 50, 65, 60, beating_heart, HEART_ANIM_FRAMES_QTY, HEART_DRAW_DELAY_US);

        if (bpm != 0 && last_bpm != bpm) {
            char bpm_buffer[5];
            snprintf(bpm_buffer, sizeof(bpm_buffer), "%ld", bpm);
            gfx_fill_rect(85, 140, 90, 35, BLACK_COLOR); // clear bpm
            gfx_draw_text(85, 140, bpm_buffer, WHITE_COLOR, 5);
        }
    } else {
        bpm = 0;

        if (last_bpm != bpm) {
            gfx_draw_icon(90, 50, heart_icon, RED_COLOR, 2);
            gfx_fill_rect(85, 140, 90, 35, BLACK_COLOR); // clear bpm

            const char *heart_rate = "--";
            gfx_draw_text(85, 140, heart_rate, WHITE_COLOR, 5);
        }
    }

    last_bpm = bpm;
}