#pragma once


typedef struct {
    int temperature;
    float wind_speed;
    int pressure;
    int humidity;
    char icon[8];
} Weather_Data;

extern const char *weather_url;
extern Weather_Data weather;

bool parse_weather();