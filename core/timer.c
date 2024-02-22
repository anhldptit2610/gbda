#include "timer.h"

int div_bit_to_freq[] = {
    [0] = 7,
    [1] = 1,
    [2] = 3,
    [3] = 5
};

uint8_t timer_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    switch (addr) {
    case TIM_REG_DIV:
        ret = (gb->tim.div >> 6) & 0x00ff;
        break;
    case TIM_REG_TAC:
        ret = gb->tim.tac.val;
        break;
    case TIM_REG_TIMA:
        ret = gb->tim.tima;
        break;
    case TIM_REG_TMA:
        ret = gb->tim.tma;
        break;
    default:
        break;
    }
    return ret;
}

void timer_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    switch (addr) {
    case TIM_REG_DIV:
        gb->tim.div = 0;
        break;
    case TIM_REG_TAC:
        gb->tim.tac.val = val;
        break;
    case TIM_REG_TIMA:
        gb->tim.tima = val;
        break;
    case TIM_REG_TMA:
        gb->tim.tma = val;
        break;
    default:
        break;
    }
}

void timer_tick(struct gb *gb)
{
    bool old_edge = gb->tim.old_edge, new_edge;

    gb->tim.div++;
    new_edge = BIT(gb->tim.div, div_bit_to_freq[gb->tim.tac.freq]);
    if (gb->tim.tac.enable & (old_edge && !new_edge)) {
        if (gb->tim.tima == 0xff) {
            // TODO: timer overflow behavior
            gb->tim.tima = gb->tim.tma;
            interrupt_request(gb, INTR_SRC_TIMER);
        } else {
            gb->tim.tima++;
        }
    } 
    gb->tim.old_edge = BIT(gb->tim.div, div_bit_to_freq[gb->tim.tac.freq]);
}