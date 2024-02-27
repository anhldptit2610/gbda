#include "dma.h"

void dma_write(struct gb *gb, uint8_t val)
{
    gb->dma.reg = val;
    gb->dma.mode = WAITING;
    gb->dma.start_addr = TO_U16(0x00, val);
}

uint8_t dma_read(struct gb *gb)
{
    return gb->dma.reg;
}

void dma_transfer(struct gb *gb)
{

}