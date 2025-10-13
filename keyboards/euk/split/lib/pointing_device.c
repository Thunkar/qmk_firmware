// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pointing_device.h"
#include "adc.h"
#include "gpio.h"
#include "timer.h"
#include "wait.h"
#include "report.h"
#include <stdlib.h>

// Separate moving average buffers for X and Y axes
static moving_avg_t moving_avg_x = {0};
static moving_avg_t moving_avg_y = {0};
static bool init = false;

// Raw joystick values (10-bit ADC)
static uint16_t raw_x = 0;
static uint16_t raw_y = 0;

// Hardware-specific ADC range with asymmetric limits (10-bit: 0-1023)
// Calibrated for single-axis range; diagonal extremes will be clamped
static const int16_t adc_min = 160;
static const int16_t adc_center = 450;
static const int16_t adc_max = 920;

static uint32_t pointing_device_timer = 0;
static uint32_t sample_joystic_timer = 0; 

void joystick_init(void) {
    if (!init) {
        moving_avg_init(&moving_avg_x);
        moving_avg_init(&moving_avg_y);
        init = true;
        gpio_set_pin_output(GP22);
        gpio_set_pin_output(GP20);
    }
}

void sample_joystick(void) {
    static bool sample_x_next = true;  // Track which axis to sample next

    // Initialize on first call
    joystick_init();

    if (timer_elapsed32(sample_joystic_timer) >= 5) {
        if (sample_x_next) {
            // Sample X and set up for Y next time
            raw_x = sample_adc(ADC_PIN, &moving_avg_x);
            gpio_write_pin_high(GP22);
            gpio_write_pin_low(GP20);
        } else {
            // Sample Y and set up for X next time
            raw_y = sample_adc(ADC_PIN, &moving_avg_y);
            gpio_write_pin_high(GP20);
            gpio_write_pin_low(GP22);
        }

        sample_x_next = !sample_x_next;
        sample_joystic_timer = timer_read32();
    }
}

void pointing_device_get_raw_values(uint16_t *x, uint16_t *y) {
    *x = raw_x;
    *y = raw_y;
}

void pointing_device_set_raw_values(uint16_t x, uint16_t y) {
    raw_x = x;
    raw_y = y;
}

// Clamp value to specified range
static int16_t clamp_value(int16_t value, int16_t min_val, int16_t max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

// Normalize ADC value to -100 to 100 range with asymmetric calibration
static int16_t normalize_adc(uint16_t raw_value, int16_t adc_min, int16_t adc_center, int16_t adc_max) {
    int16_t range_low = adc_center - adc_min;
    int16_t range_high = adc_max - adc_center;
    int16_t normalized;

    if (raw_value < adc_center) {
        // Map lower range to negative values
        normalized = ((int16_t)raw_value - adc_center) * 100 / range_low;
    } else {
        // Map upper range to positive values
        normalized = ((int16_t)raw_value - adc_center) * 100 / range_high;
    }

    // Clamp to expected range
    normalized = clamp_value(normalized, -100, 100);

    return normalized;
}

// Apply report throttling to slow down movements by skipping reports
// Returns the value to send (either original value or 0)
// skip_count: number of reports to skip (e.g., 1 = send every other report)
static int16_t apply_report_throttle(int16_t value, uint8_t skip_count, uint8_t *counter) {
    if (value == 0) {
        *counter = 0;
        return 0;
    }

    // Check if we should send this report
    if (*counter == 0) {
        *counter = skip_count + 1;  // Set to skip the next N reports
        return value;  // Send this report
    }

    // Skip this report and decrement counter
    (*counter)--;
    return 0;
}

// Apply three-stage curve: flat center, linear middle, quadratic high
static int16_t apply_curve(int16_t normalized_value) {
    const int16_t center_threshold = 75;      // Flat center region endpoint
    const int16_t high_threshold = 90;        // High acceleration zone starting point
    const int16_t center_flat_value = 35;      // Constant output value for center region

    // Handle zero case explicitly to avoid drift when centered
    if (normalized_value == 0) {
        return 0;
    }

    int16_t abs_val = abs(normalized_value);
    int16_t sign = (normalized_value < 0) ? -1 : 1;

    if (abs_val <= center_threshold) {
        // Flat center region: constant output to overcome global dampening
        return sign * center_flat_value;
    } else if (abs_val <= high_threshold) {
        // Linear middle region: interpolate from flat center value to high threshold
        int16_t range_start = center_flat_value;
        int16_t range_end = high_threshold;
        int16_t input_range = high_threshold - center_threshold;
        int16_t output_range = range_end - range_start;

        int16_t offset = abs_val - center_threshold;
        int16_t linear_value = range_start + (offset * output_range) / input_range;

        return sign * linear_value;
    } else {
        // Quadratic high region: gentle acceleration beyond the high threshold
        int16_t linear_end = high_threshold;
        int16_t excess = abs_val - high_threshold;

        // Apply gentle quadratic curve to the excess deflection
        // Scaled to provide smooth acceleration for precise fast movements
        int16_t quadratic_part = (excess * excess * 102) / 225;

        return sign * (linear_end + quadratic_part);
    }
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
    static uint8_t throttle_counter_x = 0;
    static uint8_t throttle_counter_y = 0;

    if (timer_elapsed(pointing_device_timer) < 10) {
        return mouse_report;
    }

    // Linearize to normalized range (-100 to 100) for easier math
    int16_t x_normalized = normalize_adc(raw_x, adc_min, adc_center, adc_max);
    int16_t y_normalized = normalize_adc(raw_y, adc_min, adc_center, adc_max);

    // Clamp normalized values to expected range
    x_normalized = clamp_value(x_normalized, -100, 100);
    y_normalized = clamp_value(y_normalized, -100, 100);

    // Invert Y axis (typical for joysticks)
    y_normalized = -y_normalized;

    // Apply dead zone to filter out center drift
    const int16_t dead_zone = 5;
    if (abs(x_normalized) < dead_zone) {
        x_normalized = 0;
    }
    if (abs(y_normalized) < dead_zone) {
        y_normalized = 0;
    }

    // Apply smooth quadratic curve to both axes
    int16_t x_curved = apply_curve(x_normalized);
    int16_t y_curved = apply_curve(y_normalized);

    // Apply global speed factor to control overall cursor speed
    x_curved = (int16_t)((float)x_curved * CURSOR_SPEED_FACTOR);
    y_curved = (int16_t)((float)y_curved * CURSOR_SPEED_FACTOR);

    // Clamp final value to make sure we don't under/overflow
    int16_t x = clamp_value(x_curved, -127, 127);
    int16_t y = clamp_value(y_curved, -127, 127);

    // Apply report throttling for single-pixel movements
    if (abs(x) == 1) {
        x = apply_report_throttle(x, 3, &throttle_counter_x);
    } else {
        throttle_counter_x = 0;
    }

    if (abs(y) == 1) {
        y = apply_report_throttle(y, 3, &throttle_counter_y);
    } else {
        throttle_counter_y = 0;
    }

    mouse_report.x = x;
    mouse_report.y = y;

    return mouse_report;
}