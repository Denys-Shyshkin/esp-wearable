#pragma once

#include <stdint.h>

#define ICON_COLS_QTY 32
#define ICON_ROWS_QTY 32

#define TOTAL_WEATHER_ICONS 18

typedef struct {
    const char *id;
    const uint32_t *bitmap;
} Weather_Icon_Map;

extern const uint32_t loading[ICON_ROWS_QTY];
extern const uint32_t loading_1[ICON_ROWS_QTY];
extern const uint32_t loading_2[ICON_ROWS_QTY];
extern const uint32_t loading_3[ICON_ROWS_QTY];
extern const uint32_t loading_4[ICON_ROWS_QTY];
extern const uint32_t loading_5[ICON_ROWS_QTY];

extern const uint32_t wind_icon[ICON_ROWS_QTY];
extern const uint32_t pressure_icon[ICON_ROWS_QTY];
extern const uint32_t humidity_icon[ICON_ROWS_QTY];
extern const uint32_t heart_icon[ICON_ROWS_QTY];

extern const uint32_t weather_01d[ICON_ROWS_QTY];

extern const Weather_Icon_Map weather_icons[TOTAL_WEATHER_ICONS];

const uint32_t *get_weather_icon(const char *id);