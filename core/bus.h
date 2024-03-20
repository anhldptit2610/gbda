#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"
#include "cartridge.h"
#include "sm83.h"
#include "interrupt.h"
#include "timer.h"
#include "dma.h"
#include "joypad.h"
#include "mbc.h"
#include "apu.h"

uint8_t dma_get_data(struct gb *gb, uint16_t addr);
uint8_t bus_read(struct gb *gb, uint16_t addr);
void bus_write(struct gb *gb, uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif