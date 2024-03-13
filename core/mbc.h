#pragma once

#include "gb.h"

typedef enum {
    NO_MBC,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATTERY,
    MBC2,
    MBC3,
} mbc_t;

uint8_t no_mbc_read(struct gb *gb, uint16_t addr);
uint8_t mbc1_read(struct gb *gb, uint16_t addr);
uint8_t mbc2_read(struct gb *gb, uint16_t addr);
uint8_t mbc3_read(struct gb *gb, uint16_t addr);
void no_mbc_write(struct gb *gb, uint16_t addr, uint8_t val);
void mbc1_write(struct gb *gb, uint16_t addr, uint8_t val);
void mbc1_ram_alloc(struct gb *gb);
void mbc1_ram_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t mbc1_ram_read(struct gb *gb, uint16_t addr);
void mbc1_ram_dump(struct gb *gb);
void mbc1_ram_load(struct gb *gb);
void mbc2_write(struct gb *gb, uint16_t addr, uint8_t val);
void mbc3_write(struct gb *gb, uint16_t addr, uint8_t val);
void mbc_init(struct gb *gb);