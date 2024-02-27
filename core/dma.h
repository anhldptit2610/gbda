#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gb.h"

#define DMA_REG_DMA                     0xff46
#define OAM_DMA_ADDR                    0xfe00

void dma_write(struct gb *gb, uint8_t val);
uint8_t dma_read(struct gb *gb);
void dma_transfer(struct gb *gb);

#ifdef __cplusplus
}
#endif
