#include "interrupt.h"

uint8_t interrupt_vector[] = {
    [INTR_SRC_VBLANK] = 0x40,
    [INTR_SRC_LCD] = 0x48,
    [INTR_SRC_TIMER] = 0x50,
    [INTR_SRC_SERIAL] = 0x58,
    [INTR_SRC_JOYPAD] = 0x60,
};

uint8_t interrupt_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    switch (addr) {
    case INTR_REG_IE:
        ret = gb->intr.ie;
        break;
    case INTR_REG_IF:
        ret = gb->intr.flag;
        break;
    default:
        break;
    }
    return ret;
}

void interrupt_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    switch (addr) {
    case INTR_REG_IE:
        gb->intr.ie = val | 0xe0;
        break;
    case INTR_REG_IF:
        gb->intr.flag = val | 0xe0;
        break;
    default:
        break;
    }
}

int interrupt_handler(struct gb *gb, uint8_t intr_src)
{
    gb->cpu.ime = false;
    gb->intr.flag &= ~intr_src;
    sm83_push_word(gb, gb->cpu.pc);
    gb->cpu.pc = interrupt_vector[intr_src];
    return 5;
}

void interrupt_request(struct gb *gb, uint8_t intr_src)
{
    gb->intr.flag |= intr_src;
}

bool is_interrupt_pending(struct gb *gb)
{
    return (gb->intr.ie & gb->intr.flag & 0x1f) ? 1 : 0;
}

int interrupt_process(struct gb *gb)
{
    int ret = 0;

    if (gb->cpu.ime && is_interrupt_pending(gb)) {
        if ((gb->intr.ie & gb->intr.flag & INTR_SRC_VBLANK) == INTR_SRC_VBLANK)
            ret += interrupt_handler(gb, INTR_SRC_VBLANK);
        else if ((gb->intr.ie & gb->intr.flag & INTR_SRC_LCD) == INTR_SRC_LCD)
            ret += interrupt_handler(gb, INTR_SRC_LCD);
        else if ((gb->intr.ie & gb->intr.flag & INTR_SRC_TIMER) == INTR_SRC_TIMER)
            ret += interrupt_handler(gb, INTR_SRC_TIMER);
        else if ((gb->intr.ie & gb->intr.flag & INTR_SRC_SERIAL) == INTR_SRC_SERIAL)
            ret += interrupt_handler(gb, INTR_SRC_SERIAL);
        else if ((gb->intr.ie & gb->intr.flag & INTR_SRC_JOYPAD) == INTR_SRC_JOYPAD)
            ret += interrupt_handler(gb, INTR_SRC_JOYPAD);
    }
    return ret;
}