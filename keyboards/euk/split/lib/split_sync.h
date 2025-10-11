// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

// RPC transaction structures for split sync
typedef struct _slave_to_master_t {
    uint8_t x_high;  // Upper 8 bits of x
    uint8_t x_low;   // Lower 2 bits of x (in bits 0-1)
    uint8_t y_high;  // Upper 8 bits of y
    uint8_t y_low;   // Lower 2 bits of y (in bits 0-1)
} slave_to_master_t;

typedef struct _master_to_slave_t {
    int m2s_data;
} master_to_slave_t;

// Sync joystick data with slave
void sync_with_slave(void);

// RPC handler for joystick sync
void user_pointing_device_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data);
