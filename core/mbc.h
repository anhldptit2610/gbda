#pragma once

#include "gb.h"

typedef enum {
    NO_MBC = 0x00,
    MBC1 = 0x01,
    MBC1_RAM = 0x02,
    MBC1_RAM_BATTERY = 0x03,
    MBC2 = 0x05,
    MBC2_BATTERY = 0x06,
    MBC3_TIMER_BATTERY = 0x0f,
    MBC3_TIMER_RAM_BATTERY = 0x10,
    MBC3 = 0x11,
    MBC3_RAM = 0x12,
    MBC3_RAM_BATTERY = 0x13,
} mbc_t;

/* no MBC */
uint8_t no_mbc_read(struct gb *gb, uint16_t addr);
void no_mbc_write(struct gb *gb, uint16_t addr, uint8_t val);

/* MBC1 */
uint8_t mbc1_read(struct gb *gb, uint16_t addr);
void mbc1_write(struct gb *gb, uint16_t addr, uint8_t val);
void mbc1_ram_alloc(struct gb *gb);
void mbc1_ram_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t mbc1_ram_read(struct gb *gb, uint16_t addr);
void mbc1_ram_dump(struct gb *gb);
void mbc1_ram_load(struct gb *gb);

/* MBC3 */
void mbc3_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t mbc3_read(struct gb *gb, uint16_t addr);
void mbc3_ram_dump(struct gb *gb);
void mbc3_ram_load(struct gb *gb);

void mbc_init(struct gb *gb);