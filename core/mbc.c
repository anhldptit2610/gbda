#include "mbc.h"

static uint8_t mbc1_bit_mask[] = {
    [2]   = 0b00000001,
    [4]   = 0b00000011,
    [8]   = 0b00000111,
    [16]  = 0b00001111,
    [32]  = 0b00011111,
    [64]  = 0b00011111,
    [128] = 0b00011111,
};

uint8_t no_mbc_read(struct gb *gb, uint16_t addr)
{
    return gb->cart.rom[addr];
}

void no_mbc_write(struct gb *gb, uint16_t addr, uint8_t val)
{
}

uint8_t mbc1_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff, rom_bank;

    if (IN_RANGE(addr, 0x0000, 0x3fff)) {
        ret = gb->cart.rom[addr];
    } else if (IN_RANGE(addr, 0x4000, 0x7fff)) {
        rom_bank = gb->mbc.mbc1.rom_bank_number & mbc1_bit_mask[gb->cart.infos.bank_size];
        ret = gb->cart.rom[addr - 0x4000 + 0x4000 * rom_bank];        
    }
    return ret;
}

void mbc1_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    if (IN_RANGE(addr, 0x0000, 0x1fff))
        gb->mbc.mbc1.ram_enable = val == 0x0a;
    else if (IN_RANGE(addr, 0x2000, 0x3fff))
        gb->mbc.mbc1.rom_bank_number = (!(val & 0x1f)) ? 1 : val & 0x1f;
    else if (IN_RANGE(addr, 0x4000, 0x5fff))
        gb->mbc.mbc1.ram_bank_number = val & 0x03; 
    else if (IN_RANGE(addr, 0x6000, 0x7fff))
        gb->mbc.mbc1.banking_mode = BIT(val, 0);
}