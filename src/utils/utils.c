#include <stdint.h>
#include "utils.h"

#define MAX_WINDOW_SIZE 100

uint32_t utils_moving_average(int *raw, uint8_t window_size) {
    static uint32_t sample_buffer[MAX_WINDOW_SIZE] = {0};
    static uint8_t buffer_index = 0;
    static uint8_t samples_filled = 0;
    static uint32_t running_sum = 0;

    if (window_size > MAX_WINDOW_SIZE || window_size == 0) {
        return *raw; 
    }

    running_sum -= sample_buffer[buffer_index];
    sample_buffer[buffer_index] = *raw;
    running_sum += *raw;

    buffer_index++;
    if (buffer_index >= window_size) {
        buffer_index = 0;
    }

    if (samples_filled < window_size) {
        samples_filled++;
    }

    return running_sum / samples_filled;
}