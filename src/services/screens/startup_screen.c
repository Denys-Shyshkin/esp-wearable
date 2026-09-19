#include "config/config.h"
#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"

void startup_screen(enum Screen_Event event, bool func_status[]) {
    if (event == ENTER) {
        display_clear();

        const char *startup = "STARTING";
        gfx_draw_text(60, 10, startup, SEA_GREEN_COLOR, 2);
    } else {
        static bool is_displayed = false;

        if (!is_displayed) {
            gfx_draw_icon(200, 30, start, GREEN_COLOR, 1);
            gfx_fill_rect(235, 30, 2, 30, GREEN_COLOR);


            bool has_fails = false;
            for (int i = 0; i < FUNC_TOTAL_COUNT; i++) {
                if (!func_status[i]) {
                    has_fails = true;
                }
            }

            if (has_fails) {
                gfx_draw_icon(200, 190, reset, RED_COLOR, 1);
                gfx_fill_rect(235, 190, 2, 30, RED_COLOR);
            }

            is_displayed = true;
        }
    }
}