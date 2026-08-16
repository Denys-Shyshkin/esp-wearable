#pragma once

#include <stdint.h>

#define RESPONSE_BUFFER_SIZE 12000

extern char response_buffer[RESPONSE_BUFFER_SIZE];
extern size_t response_len;

bool http_get(const char *url);