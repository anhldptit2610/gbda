#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"

void cartridge_load(struct gb *gb, char *cartridge_path);
void load_state_after_booting(struct gb *gb);
void rom_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t rom_read(struct gb *gb, uint16_t addr);

#ifdef __cplusplus
}
#endif
