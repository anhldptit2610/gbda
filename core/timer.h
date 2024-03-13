#pragma once

#include "gb.h"
#include "interrupt.h"

struct gb;

#define TIM_REG_DIV                 0xff04
#define TIM_REG_TIMA                0xff05
#define TIM_REG_TMA                 0xff06
#define TIM_REG_TAC                 0xff07

uint8_t timer_read(struct gb *gb, uint16_t addr);
void timer_write(struct gb *gb, uint16_t addr, uint8_t val);
void timer_tick(struct gb *gb);