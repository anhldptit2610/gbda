#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"
#include "bus.h"

void sm83_step(struct gb *gb);
void sm83_init(struct gb *gb);
void sm83_cycle(struct gb *gb);

#ifdef __cplusplus
}
#endif