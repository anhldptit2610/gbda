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
        ret = (gb->timer.div >> 6) & 0x00ff;
        break;
    case TIM_REG_TAC:
        ret = gb->timer.tac.val;
        break;
    case TIM_REG_TIMA:
        ret = gb->timer.tima;
        break;
    case TIM_REG_TMA:
        ret = gb->timer.tma;
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
        gb->timer.div = 0;
        break;
    case TIM_REG_TAC:
        gb->timer.tac.val = val;
        break;
    case TIM_REG_TIMA:
        gb->timer.tima = val;
        break;
    case TIM_REG_TMA:
        gb->timer.tma = val;
        break;
    default:
        break;
    }
}

void timer_tick(struct gb *gb)
{
    bool old_edge = gb->timer.old_edge, new_edge;

    gb->timer.div++;
    new_edge = BIT(gb->timer.div, div_bit_to_freq[gb->timer.tac.freq]);
    if (gb->timer.tac.enable & (old_edge && !new_edge)) {
        if (gb->timer.tima == 0xff) {
            // TODO: timer overflow behavior
            gb->timer.tima = gb->timer.tma;
            interrupt_request(gb, INTR_SRC_TIMER);
        } else {
            gb->timer.tima++;
        }
    } 
    gb->timer.old_edge = BIT(gb->timer.div, div_bit_to_freq[gb->timer.tac.freq]);
}