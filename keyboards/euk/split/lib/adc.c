// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "adc.h"
#include "analog.h"
#include <stdlib.h>

// qsort requires you to create a sort function
static int sort_desc(const void *cmp1, const void *cmp2) {
    // Cast to uint16_t for full 10-bit ADC range
    uint16_t a = *((uint16_t *)cmp1);
    uint16_t b = *((uint16_t *)cmp2);
    // The comparison
    return a > b ? -1 : (a < b ? 1 : 0);
}

// Initialize moving average buffer
void moving_avg_init(moving_avg_t *avg) {
    for (int i = 0; i < MOVING_AVG_SIZE; i++) {
        avg->buffer[i] = 0;
    }
    avg->index = 0;
    avg->count = 0;
    avg->sum = 0;
}

// Add new sample to moving average and return the current average
uint16_t moving_avg_update(moving_avg_t *avg, uint16_t new_sample) {
    // Remove old value from sum if buffer is full
    if (avg->count == MOVING_AVG_SIZE) {
        avg->sum -= avg->buffer[avg->index];
    } else {
        avg->count++;
    }

    // Add new value
    avg->buffer[avg->index] = new_sample;
    avg->sum += new_sample;

    // Move to next position (circular buffer)
    avg->index = (avg->index + 1) % MOVING_AVG_SIZE;

    // Return average
    return (uint16_t)(avg->sum / avg->count);
}

uint16_t sample_adc(const int pin, moving_avg_t *moving_avg) {
    uint16_t measurements[ADC_SAMPLES];

    for (int i = 0; i < ADC_SAMPLES; i++) {
        measurements[i] = analogReadPin(pin);  // Full 10-bit range (0-1023)
    }

    qsort(measurements, ADC_SAMPLES, sizeof(measurements[0]), sort_desc);

    // Remove top and bottom 20% of samples as outliers
    const int outliers_per_side = ADC_SAMPLES / 5;  // 20%
    const int start_idx = outliers_per_side;
    const int end_idx = ADC_SAMPLES - outliers_per_side;
    const int valid_samples = end_idx - start_idx;

    uint32_t sum = 0;
    for (int i = start_idx; i < end_idx; i++) {
        sum += measurements[i];
    }

    uint16_t current_sample = (uint16_t)(sum / (uint32_t)valid_samples);

    // Apply moving average filter to smooth transitions
    return moving_avg_update(moving_avg, current_sample);
}
