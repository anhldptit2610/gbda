#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"
#include "bus.h"
#include "interrupt.h"
#include "timer.h"

void sm83_step(struct gb *gb);
void sm83_init(struct gb *gb);
void sm83_cycle(struct gb *gb);
void sm83_push_word(struct gb *gb, uint16_t val);

#ifdef __cplusplus
}
#endif