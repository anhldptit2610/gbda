#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"
#include "interrupt.h"

#define PPU_REG_LCDC                0xff40
#define PPU_REG_STAT                0xff41
#define PPU_REG_SCY                 0xff42
#define PPU_REG_SCX                 0xff43
#define PPU_REG_LY                  0xff44
#define PPU_REG_LYC                 0xff45
#define PPU_REG_BGP                 0xff47
#define PPU_REG_OBP0                0xff48
#define PPU_REG_OBP1                0xff49
#define PPU_REG_WY                  0xff4a
#define PPU_REG_WX                  0xff4b

#define STAT_INTR_MODE0             (1U << 3)
#define STAT_INTR_MODE1             (1U << 4)
#define STAT_INTR_MODE2             (1U << 5)
#define STAT_INTR_LYC               (1U << 6)

typedef enum {
    OBP0,
    OBP1,
    BGP,
} palette_t;

typedef enum {
    BG_WIN,
    SPRITE,
} pixel_type_t;

uint8_t ppu_read(struct gb *gb, uint16_t addr);
void ppu_write(struct gb *gb, uint16_t addr, uint8_t val);
void ppu_tick(struct gb *gb);

#ifdef __cplusplus
}
#endif