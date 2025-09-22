// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "print.h"
#include "analog.h"
#include <transactions.h>

#define POT_PIN GP26

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
                                                            KC_SPC  , KC_LEFT ,       KC_NUBS , KC_SPC
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

uint8_t last_val = 0;
uint32_t last_sync = 0;
uint8_t pressed = 0;    
uint8_t last_pressed = 0;

typedef struct _slave_to_master_t {
    uint8_t s2m_data;
} slave_to_master_t;

typedef struct _master_to_slave_t {
    int m2s_data;
} master_to_slave_t;

void user_encoder_press_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    slave_to_master_t *s2m = (slave_to_master_t*)out_data;
    s2m->s2m_data = pressed;
    pressed = 0;
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(ENCODER_PRESS_SYNC, user_encoder_press_sync_handler);
}

void sync_with_slave(void) {
    if(timer_elapsed32(last_sync) > 100) {
        master_to_slave_t m2s = {0};
        slave_to_master_t s2m = {0};
        if(transaction_rpc_exec(ENCODER_PRESS_SYNC, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            last_sync = timer_read32();
            pressed = s2m.s2m_data;
            if(pressed && !last_pressed) {
                tap_code(KC_MUTE);
            }
            last_pressed = pressed;
        } else {
            uprintf("Slave sync failed!\n");
        }
    }
}

void check_pot(void) {
    uint8_t current_val = analogReadPin(POT_PIN) >>2;
    int change = last_val - current_val;
    if(abs(change) < 5) return;
    hsv_t current_hsv = rgb_matrix_get_hsv();
    rgb_matrix_sethsv_noeeprom(current_hsv.h, current_hsv.s, current_val);
    last_val = current_val;
}

void check_encoder_push(void) {
    if(timer_elapsed32(last_sync) > 100) {
        uint8_t current_val = analogReadPin(POT_PIN) >>2;;
        pressed = current_val < 10 && last_val < 10;
        last_sync = timer_read32(); 
        last_val = current_val;
    }
}

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        sync_with_slave();
        check_pot();
    } else {
        check_encoder_push();
    } 
}

bool oled_task_user(void) {
    oled_clear();
    oled_set_cursor(0,10);
    oled_write_P(PSTR("B:"), false);
    char val_str[3];
    itoa(last_val, val_str, 10);
    oled_write(val_str, false);
    return false;
}