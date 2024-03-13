#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"
#include "sm83.h"

#define INTR_REG_IE         0xffff
#define INTR_REG_IF         0xff0f

#define INTR_SRC_VBLANK     (1U << 0)
#define INTR_SRC_LCD        (1U << 1)
#define INTR_SRC_TIMER      (1U << 2)
#define INTR_SRC_SERIAL     (1U << 3)
#define INTR_SRC_JOYPAD     (1U << 4)

uint8_t interrupt_read(struct gb *gb, uint16_t addr);
void interrupt_write(struct gb *gb, uint16_t addr, uint8_t val);
int interrupt_process(struct gb *gb);
void interrupt_request(struct gb *gb, uint8_t intr_src);
bool is_interrupt_pending(struct gb *gb);

#ifdef __cplusplus
}
#endif