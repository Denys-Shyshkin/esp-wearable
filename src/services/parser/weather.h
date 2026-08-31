#pragma once


typedef struct {
    int temperature;
    float wind_speed;
    int pressure;
    int humidity;
    char icon[8];
} weather_data;

extern const char *weather_url;
extern weather_data weather;

bool parse_weather();