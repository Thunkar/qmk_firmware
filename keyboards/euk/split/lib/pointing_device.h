// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

#define ADC_PIN GP26
#define CURSOR_SPEED_FACTOR 0.03f  // Global speed multiplier (0.0-1.0). Adjust to reduce/increase overall speed

// Initialize joystick sampling (called automatically)
void joystick_init(void);

// Sample joystick on slave side (alternates between X and Y)
void sample_joystick(void);

// Get raw joystick values (used by slave to sync)
void pointing_device_get_raw_values(uint16_t *x, uint16_t *y);

// Set raw joystick values (used by master after sync)
void pointing_device_set_raw_values(uint16_t x, uint16_t y);