#include "config/config.h"
#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/graphics.h"
#include "services/graphics/icons.h"

void startup_screen(enum Screen_Event event, bool func_status[]) {
    if (event == ENTER) {
        display_clear();

        const char *startup = "STARTING";
        gfx_draw_text(60, 10, startup, COLOR_SECONDARY, 2);
    } else {
        static bool is_controls_displayed = false;

        if (!is_controls_displayed) {
            gfx_draw_icon(200, 30, start, COLOR_SUCCESS, 1);
            gfx_fill_rect(235, 30, 2, 30, COLOR_SUCCESS);


            bool has_fails = false;
            for (int i = 0; i < FUNC_TOTAL_COUNT; i++) {
                if (!func_status[i]) {
                    has_fails = true;
                }
            }

            if (has_fails) {
                gfx_draw_icon(200, 190, reset, COLOR_ERROR, 1);
                gfx_fill_rect(235, 190, 2, 30, COLOR_ERROR);
            }

            is_controls_displayed = true;
        }
    }
}