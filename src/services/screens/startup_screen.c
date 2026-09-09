#include "drivers/display/display.h"
#include "screen_manager.h"
#include "services/graphics/graphics.h"

void startup_screen(enum Screen_Event event) {
    if (event == ENTER) {
        display_clear();

        const char *startup = "STARTING";
        gfx_draw_text(60, 20, startup, SEA_GREEN_COLOR, 2);
    }
}