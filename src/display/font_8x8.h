#pragma once

#include <stdint.h>

#define FONT_8X8_FIRST_CHAR '!'
#define FONT_8X8_LAST_CHAR '~'
#define FONT_8X8_CHAR_QTY 94 // number of printable ASCII characters
#define FONT_8X8_ROWS_QTY 8  // rows that make up each glyph
#define FONT_8X8_COLS_QTY 8  // cols that make up each glyph
#define FONT_8X8_SPACING 0   // spacing between each char

extern const uint8_t font_8x8[FONT_8X8_CHAR_QTY][FONT_8X8_ROWS_QTY];