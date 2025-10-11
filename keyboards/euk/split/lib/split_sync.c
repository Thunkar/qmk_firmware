// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "split_sync.h"
#include "pointing_device.h"
#include "print.h"
#include <transactions.h>
#include "timer.h"

static uint32_t pointing_device_timer = 0;

// Called by the slave to respond to the master's request
void user_pointing_device_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    slave_to_master_t *s2m = (slave_to_master_t*)out_data;
    uint16_t raw_x, raw_y;

    pointing_device_get_raw_values(&raw_x, &raw_y);

    // Split 10-bit values into high and low bytes
    s2m->x_high = (uint8_t)(raw_x >> 2);   // Upper 8 bits
    s2m->x_low = (uint8_t)(raw_x & 0x03);  // Lower 2 bits
    s2m->y_high = (uint8_t)(raw_y >> 2);   // Upper 8 bits
    s2m->y_low = (uint8_t)(raw_y & 0x03);  // Lower 2 bits
}

// Called by the master to get the pointing device values
void sync_with_slave(void) {
    if(timer_elapsed32(pointing_device_timer) > 20) {
        master_to_slave_t m2s = {0};
        slave_to_master_t s2m = {0, 0, 0, 0};
        if(transaction_rpc_exec(POINTING_DEVICE_SYNC, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            pointing_device_timer = timer_read32();
            // Reassemble 10-bit values from high and low bytes
            uint16_t raw_x = (((uint16_t)s2m.x_high) << 2) | s2m.x_low;
            uint16_t raw_y = (((uint16_t)s2m.y_high) << 2) | s2m.y_low;
            pointing_device_set_raw_values(raw_x, raw_y);
        } else {
            uprintf("Slave sync failed!\n");
        }
    }
}
