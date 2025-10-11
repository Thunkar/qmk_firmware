// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "encoder.h"
#include "analog.h"
#include "timer.h"
#include "action.h"

static uint32_t encoder_timer = 0;
static uint8_t last_val = 0;
static uint8_t pressed = 0;
static uint8_t last_pressed = 0;

void check_encoder_push(void) {
    if(timer_elapsed32(encoder_timer) > 100) {
        uint8_t current_val = analogReadPin(ADC_PIN) >>2;
        pressed = current_val < 10 && last_val < 10;
        encoder_timer = timer_read32();
        last_val = current_val;
        if(pressed && !last_pressed) {
            tap_code(KC_MUTE);
        }
        last_pressed = pressed;
    }
}
