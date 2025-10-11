// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "print.h"
#include "analog.h"
#include <transactions.h>

#define ADC_PIN GP26
#define ADC_SAMPLES 50

enum layers {
    _QWERTY = 0,
    _UTIL,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* QWERTY (Spanish)
    * ,-----------------------------------------.                                  ,-----------------------------------------.
    * | ESC  |   1  |   2  |   3  |   4  |   5  |                                  |   6  |   7  |   8  |   9  |   0  | Bcksp|
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * | Tab  |   Q  |   W  |   E  |   R  |   T  |                                  |   Y  |   U  |   I  |   O  |   P  | { [  |
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * | LWin |   A  |   S  |   D  |   F  |   G  |                                  |   H  |   J  |   K  |   L  |   ;  | " '  |
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * |LShft |   Z  |   X  |   C  |   V  |   B  |                                  |   N  |   M  |  < , | > .  | ? /  | } ]  |
    * |------+------+------+------+------+------/                                  `------+------+------+------+------+------|
    * | LCtl | LWin | LAlt | MO(2)| MO(1)|       ,------·------·    ,------·------·       | Entr | RWin | RAlt | Home | Del  |
    * `----------------------------------/       | Scr- | Scr+ |    | Vol- | Vol+ |       \----------------------------------'
    *                                            |------+------|    |------+------|       
    *                                            |Space |      |    | NUBS |Space |
    *                                            `-------------'    `-------------'                                            
    */                                           

    [_QWERTY] = LAYOUT(
      KC_ESC  , KC_1    ,  KC_2   ,  KC_3  ,  KC_4  ,  KC_5 ,                                           KC_6  ,  KC_7  ,  KC_8    , KC_9    , KC_0     ,  KC_BSPC ,         
      KC_TAB  , KC_Q    ,  KC_W   ,  KC_E  ,  KC_R  ,  KC_T ,                                           KC_Y  ,  KC_U  ,  KC_I    , KC_O    , KC_P     ,  KC_LBRC ,        
      KC_LGUI , KC_A    ,  KC_S   ,  KC_D  ,  KC_F  ,  KC_G ,                                           KC_H  ,  KC_J  ,  KC_K    , KC_L    , KC_SCLN  ,  KC_QUOT ,         
      KC_LSFT , KC_Z    ,  KC_X   ,  KC_C  ,  KC_V  ,  KC_B ,                                           KC_N  ,  KC_M  ,  KC_COMM , KC_DOT  , KC_SLSH  ,  KC_RBRC ,         
      KC_LCTL , KC_LGUI ,  KC_LALT,  MO(2) ,  MO(1) ,                                                           KC_ENT ,  KC_RGUI , KC_RALT , KC_HOME  ,  KC_DEL  ,
                                                            KC_BRID , KC_BRIU,        KC_VOLD , KC_VOLU,                     
                                                            KC_SPC  , _______ ,       KC_NUBS , KC_SPC
    ),

    /* Util
    * ,-----------------------------------------.                                  ,-----------------------------------------.
    * | ~ `  |  F1  |  F2  |  F3  |  F4  |  F5  |                                  |  F6  |  F7  |  F8  |  F9  | F10  | - _  |
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * |      |      |      |      |      |      |                                  |      |      | Up   |      |  [   |   ]  |
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * |      |      |      |      |      |      |                                  |      | Left | Down | Right|  {   |   }  |
    * |------+------+------+------+------+------|                                  |------+------+------+------+------+------|
    * |      |      |      |      |      |      |                                  |      |      |      |      |      |      |
    * |------+------+------+------+------+------/                                  `------+------+------+------+------+------|
    * |      |      |      |      |      |       ,------·------·    ,------·------·       |      |      |      | F11  | F12  |
    * `----------------------------------/       |      |      |    |      |      |       \----------------------------------'
    *                                            |------+------|    |------+------|       
    *                                            |      |      |    |      |      |
    *                                            `-------------'    `-------------'                                            
    */                                           

    [_UTIL] = LAYOUT(
      KC_GRV  , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   ,                                            KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_MINS,         
      _______ , _______ , _______ , _______ , _______ , _______ ,                                            _______ , _______ , KC_UP   , _______ , RALT(KC_LBRC) , RALT(KC_RBRC),        
      _______ , _______ , _______ , _______ , _______ , _______ ,                                            _______ , KC_LEFT , KC_DOWN , KC_RIGHT, RALT(KC_QUOT) , RALT(KC_NUHS),         
      _______ , _______ , _______ , _______ , _______ , _______ ,                                            _______ , _______ , _______ , _______ , _______ , _______ ,         
      _______ , _______ , _______ , _______ , _______ ,                                                                _______ , _______ , _______ , KC_F11  , KC_F12  ,
                                                                _______ , _______ ,       _______  , _______ ,                    
                                                                _______ , _______ ,       _______  , _______
    ),
};
// clang-format on

// qsort requires you to create a sort function
int sort_desc(const void *cmp1, const void *cmp2) {
  // Cast to uint16_t for full 10-bit ADC range
  uint16_t a = *((uint16_t *)cmp1);
  uint16_t b = *((uint16_t *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
}

uint16_t sample_adc(const int pin) {
  uint16_t measurements[ADC_SAMPLES];

  for (int i = 0; i < ADC_SAMPLES; i++) {
    measurements[i] = analogReadPin(ADC_PIN);  // Full 10-bit range (0-1023)
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

  return (uint16_t)(sum / (uint32_t)valid_samples);
}

// Encoder
uint32_t encoder_timer = 0;
uint8_t last_val = 0;
uint8_t pressed = 0;
uint8_t last_pressed = 0;

// Joystick
uint32_t joystick_timer = 0;
uint16_t raw_x = 0;
uint16_t raw_y = 0;
int16_t x_normalized = 0;
int16_t y_normalized = 0;
int16_t x = 0;
int16_t y = 0;

typedef struct _slave_to_master_t {
    uint8_t x_high;  // Upper 8 bits of x
    uint8_t x_low;   // Lower 2 bits of x (in bits 0-1)
    uint8_t y_high;  // Upper 8 bits of y
    uint8_t y_low;   // Lower 2 bits of y (in bits 0-1)
} slave_to_master_t;

typedef struct _master_to_slave_t {
    int m2s_data;
} master_to_slave_t;

void user_joystick_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    slave_to_master_t *s2m = (slave_to_master_t*)out_data;
    // Split 10-bit values into high and low bytes
    s2m->x_high = (uint8_t)(raw_x >> 2);   // Upper 8 bits
    s2m->x_low = (uint8_t)(raw_x & 0x03);  // Lower 2 bits
    s2m->y_high = (uint8_t)(raw_y >> 2);   // Upper 8 bits
    s2m->y_low = (uint8_t)(raw_y & 0x03);  // Lower 2 bits
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(JOYSTICK_SYNC, user_joystick_sync_handler);
    if(!is_keyboard_master()) {
        gpio_set_pin_output(GP22);    
        gpio_set_pin_output(GP20);
    }
}

void sync_with_slave(void) {
    if(timer_elapsed32(joystick_timer) > 100) {
        master_to_slave_t m2s = {0};
        slave_to_master_t s2m = {0, 0, 0, 0};
        if(transaction_rpc_exec(JOYSTICK_SYNC, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            joystick_timer = timer_read32();
            // Reassemble 10-bit values from high and low bytes
            raw_x = (((uint16_t)s2m.x_high) << 2) | s2m.x_low;
            raw_y = (((uint16_t)s2m.y_high) << 2) | s2m.y_low;
        } else {
            uprintf("Slave sync failed!\n");
        }
    }
}

void check_pot(void) {
    uint8_t current_val = analogReadPin(ADC_PIN) >>2;
    if (current_val < 6) current_val = 0;
    if (current_val > 249) current_val = 255;
    int change = last_val - current_val; 
    if(abs(change) < 5) return;
    hsv_t current_hsv = rgb_matrix_get_hsv();
    rgb_matrix_sethsv_noeeprom(current_hsv.h, current_hsv.s, current_val);
    last_val = current_val;
}

void sample_joystick(void) {
    gpio_write_pin_high(GP20);
    gpio_write_pin_low(GP22);
    wait_ms(10);
    raw_x = sample_adc(ADC_PIN);
    gpio_write_pin_high(GP22);
    gpio_write_pin_low(GP20);
    wait_ms(10);
    raw_y = sample_adc(ADC_PIN);
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
    if (timer_elapsed(joystick_timer) < 10) {
        wait_ms(2);
        return mouse_report;
    }

    // Hardware-specific ADC range with asymmetric limits (10-bit: 0-1023)
    // Calibrated for single-axis range; diagonal extremes will be clamped
    const int16_t adc_min = 120;
    const int16_t adc_center = 450;
    const int16_t adc_max = 930;
    const int16_t range_low = adc_center - adc_min;
    const int16_t range_high = adc_max - adc_center;

    // Linearize to normalized range (-100 to 100) for easier math
    if (raw_x < adc_center) {
        // Map lower range to negative values
        x_normalized = ((int16_t)raw_x - adc_center) * 100 / range_low;
    } else {
        // Map upper range to positive values
        x_normalized = ((int16_t)raw_x - adc_center) * 100 / range_high;
    }

    if (raw_y < adc_center) {
        // Map lower range to negative values
        y_normalized = ((int16_t)raw_y - adc_center) * 100 / range_low;
    } else {
        // Map upper range to positive values
        y_normalized = ((int16_t)raw_y - adc_center) * 100 / range_high;
    }

    // Clamp normalized values to expected range
    if (x_normalized < -100) x_normalized = -100;
    if (x_normalized > 100) x_normalized = 100;
    if (y_normalized < -100) y_normalized = -100;
    if (y_normalized > 100) y_normalized = 100;

    // Invert Y axis (typical for joysticks)
    y_normalized = -y_normalized;

    // Apply dead zone to filter out center drift
    const int16_t dead_zone = 1;
    if (abs(x_normalized) < dead_zone) {
        x_normalized = 0;
    }
    if (abs(y_normalized) < dead_zone) {
        y_normalized = 0;
    }

    // Apply hybrid curve: dampened linear up to threshold, then quadratic
    // Provides fine control near center, smooth acceleration at edges
    const int16_t linear_threshold = 30;
    const int16_t linear_dampening = 10;
    int16_t x_curved, y_curved;

    // Process X axis
    if (abs(x_normalized) <= linear_threshold) {
        // Linear region with dampening for fine control
        x_curved = x_normalized / linear_dampening;
    } else {
        // Quadratic region: smooth acceleration beyond threshold
        int16_t excess = abs(x_normalized) - linear_threshold;
        int16_t quadratic_part = (excess * excess) / 50;  // Scaled quadratic
        x_curved = (x_normalized < 0 ? -1 : 1) * ((linear_threshold / linear_dampening) + quadratic_part);
    }

    // Process Y axis
    if (abs(y_normalized) <= linear_threshold) {
        // Linear region with dampening for fine control
        y_curved = y_normalized / linear_dampening;
    } else {
        // Quadratic region: smooth acceleration beyond threshold
        int16_t excess = abs(y_normalized) - linear_threshold;
        int16_t quadratic_part = (excess * excess) / 50;  // Scaled quadratic
        y_curved = (y_normalized < 0 ? -1 : 1) * ((linear_threshold / linear_dampening) + quadratic_part);
    }

    // Cap magnitude for consistent speed in all directions
    // Prevents diagonal movement from being faster than cardinal directions
    const int16_t max_speed = 4;
    int16_t x_limited = x_curved;
    int16_t y_limited = y_curved;
    if (x_curved != 0 || y_curved != 0) {
        // Fast integer approximation of magnitude
        int16_t abs_x = abs(x_curved);
        int16_t abs_y = abs(y_curved);
        int16_t magnitude = (abs_x > abs_y) ? (abs_x + abs_y / 2) : (abs_y + abs_x / 2);

        if (magnitude > max_speed) {
            // Scale both components down proportionally
            x_limited = (x_curved * max_speed) / magnitude;
            y_limited = (y_curved * max_speed) / magnitude;
        }
    }

    // Clamp final value to make sure we don't under/overflow
    if (y_limited < -127) { y_limited = -127; }
    if (y_limited > 127) { y_limited = 127; }
    if (x_limited < -127) { x_limited = -127; }
    if (x_limited > 127) { x_limited = 127; }

    x = x_limited;
    y = y_limited;

    mouse_report.x = x;
    mouse_report.y = y;

    return mouse_report;
}

void pointing_device_driver_init(void) {
}

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

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        sync_with_slave();
        check_encoder_push();
    } else {
        sample_joystick();
    } 
}

// Rotate OLED
// Image MUST be converted to VERTICAL on image2cpp*******
oled_rotation_t oled_init_user(oled_rotation_t rotation) { 
    return OLED_ROTATION_90;
}

// Render left image
static void render_left(void) {
    oled_clear();
    char buf[8];

    oled_write_ln_P(PSTR("JOY"), false);
    oled_write_ln_P(PSTR(""), false);

    // Normalized X (-100 to 100)
    oled_write_ln_P(PSTR("NX"), false);
    snprintf(buf, sizeof(buf), "%d", x_normalized);
    oled_write_ln(buf, false);
    oled_write_ln_P(PSTR(""), false);

    // Normalized Y (-100 to 100)
    oled_write_ln_P(PSTR("NY"), false);
    snprintf(buf, sizeof(buf), "%d", y_normalized);
    oled_write_ln(buf, false);
    oled_write_ln_P(PSTR(""), false);

    // Mouse X (-127 to 127)
    oled_write_ln_P(PSTR("MX"), false);
    snprintf(buf, sizeof(buf), "%d", x);
    oled_write_ln(buf, false);

    // Mouse Y (-127 to 127)
    oled_write_ln_P(PSTR("MY"), false);
    snprintf(buf, sizeof(buf), "%d", y);
    oled_write_ln(buf, false);
}


static void render_right(void) {
    oled_clear();
    char buf[8];

    oled_write_ln_P(PSTR("JOY"), false);
    oled_write_ln_P(PSTR(""), false);

    // Raw X (10-bit)
    oled_write_ln_P(PSTR("RX"), false);
    snprintf(buf, sizeof(buf), "%d", raw_x);
    oled_write_ln(buf, false);
    oled_write_ln_P(PSTR(""), false);

    // Raw Y (10-bit)
    oled_write_ln_P(PSTR("RY"), false);
    snprintf(buf, sizeof(buf), "%d", raw_y);
    oled_write_ln(buf, false);
    oled_write_ln_P(PSTR(""), false);

    // Mouse X
    oled_write_ln_P(PSTR("MX"), false);
    snprintf(buf, sizeof(buf), "%d", x);
    oled_write_ln(buf, false);

    // Mouse Y
    oled_write_ln_P(PSTR("MY"), false);
    snprintf(buf, sizeof(buf), "%d", y);
    oled_write_ln(buf, false);
}

// Draw to OLED
bool oled_task_user() {
    if (is_keyboard_master()) {
        render_left();
    } else {
        render_right();
    }

    return false;
}