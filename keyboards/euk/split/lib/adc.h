// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

// ADC sampling configuration
#define ADC_SAMPLES 50
#define MOVING_AVG_SIZE 10  // Number of samples for moving average filter

// Moving average buffer structure
typedef struct {
    uint16_t buffer[MOVING_AVG_SIZE];
    uint8_t index;
    uint8_t count;
    uint32_t sum;
} moving_avg_t;

// Initialize moving average buffer
void moving_avg_init(moving_avg_t *avg);

// Add new sample to moving average and return the current average
uint16_t moving_avg_update(moving_avg_t *avg, uint16_t new_sample);

// Sample ADC with outlier filtering and moving average
uint16_t sample_adc(const int pin, moving_avg_t *moving_avg);
