#pragma once

#include "gb.h"
#include "interrupt.h"

#define JOYPAD_REG_JOYP             0xff00

typedef enum JOYPAD_KEYS {
    JOYPAD_A = (1U << 0),
    JOYPAD_B = (1U << 1),
    JOYPAD_SELECT = (1U << 2),
    JOYPAD_START = (1U << 3),
    JOYPAD_RIGHT = (1U << 4),
    JOYPAD_LEFT = (1U << 5),
    JOYPAD_UP = (1U << 6),
    JOYPAD_DOWN = (1U << 7),
} joypad_keys_t;

uint8_t joypad_read(struct gb *gb);
void joypad_write(struct gb *gb, uint8_t val);
void joypad_press_button(struct gb *gb, joypad_keys_t key);
void joypad_release_button(struct gb *gb, joypad_keys_t key);
