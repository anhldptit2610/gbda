#include "joypad.h"

uint8_t joypad_read(struct gb *gb)
{
    uint8_t ret = 0xff;

    if (gb->joypad.joyp.select_button && gb->joypad.joyp.select_dpad)
        ret = 0xff;
    else if (!gb->joypad.joyp.select_dpad)
        ret = (gb->joypad.joyp.val & 0xf0) | (gb->joypad.right << 0) | (gb->joypad.left << 1) | (gb->joypad.up << 2) | (gb->joypad.down << 3);
    else if (!gb->joypad.joyp.select_button)
        ret = (gb->joypad.joyp.val & 0xf0) | (gb->joypad.a << 0) | (gb->joypad.b << 1) | (gb->joypad.select << 2) | (gb->joypad.start << 3);
    return ret;
}

void joypad_write(struct gb *gb, uint8_t val)
{
    gb->joypad.joyp.val = val | (gb->joypad.joyp.val & 0xcf) | 0xc0;
}

void joypad_press_button(struct gb *gb, joypad_keys_t key)
{
    switch (key) {
    case JOYPAD_A:
        gb->joypad.a = 0;
        break;
    case JOYPAD_B:
        gb->joypad.b = 0;
        break;
    case JOYPAD_SELECT:
        gb->joypad.select = 0;
        break;
    case JOYPAD_START:
        gb->joypad.start = 0;
        break;
    case JOYPAD_RIGHT:    
        gb->joypad.right = 0;
        break;
    case JOYPAD_LEFT:
        gb->joypad.left = 0;
        break;
    case JOYPAD_UP:
        gb->joypad.up = 0;
        break;
    case JOYPAD_DOWN:
        gb->joypad.down = 0;
        break;
    default:
        break;
    }
    interrupt_request(gb, INTR_SRC_JOYPAD);
}

void joypad_release_button(struct gb *gb, joypad_keys_t key)
{
    switch (key) {
    case JOYPAD_A:
        gb->joypad.a = 1;
        break;
    case JOYPAD_B:
        gb->joypad.b = 1;
        break;
    case JOYPAD_SELECT:
        gb->joypad.select = 1;
        break;
    case JOYPAD_START:
        gb->joypad.start = 1;
        break;
    case JOYPAD_RIGHT:    
        gb->joypad.right = 1;
        break;
    case JOYPAD_LEFT:
        gb->joypad.left = 1;
        break;
    case JOYPAD_UP:
        gb->joypad.up = 1;
        break;
    case JOYPAD_DOWN:
        gb->joypad.down = 1;
        break;
    default:
        break;
    }
}