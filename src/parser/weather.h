#pragma once

extern const char *weather_url;

typedef struct {
    float temperature;
    float feels_like;
    int humidity;
    int pressure;
} Weather_Data;

void parse_weather();