#include "bus.h"

enum BUS_REGION {
    ROM = 1,
    RAM = 2,
    OAM = 4,
    IO  = 8,
    UNUSED = 16,
};

/* write functions */
void ram_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    if (addr >= 0xe000 && addr <= 0xfdff)
        addr &= 0xddff;

    if (addr >= 0x8000 && addr <= 0x9fff)
        gb->vram[addr - 0x8000] = val;
    else if (addr >= 0xa000 && addr <= 0xbfff)
        gb->eram[addr - 0xa000] = val;
    else if (addr >= 0xc000 && addr <= 0xdfff)
        gb->wram[addr - 0xc000] = val;
    else if (addr >= 0xff80 && addr <= 0xfffe)
        gb->hram[addr - 0xff80] = val;
}

void oam_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    gb->oam[addr - 0xfe00] = val; 
}

void io_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    // TODO: complete this function. Now just treat IO memory 
    //       range like an array 
}

void unused_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    
}

/* write functions */
uint8_t ram_read(struct gb *gb, uint16_t addr)
{

}

uint8_t oam_read(struct gb *gb, uint16_t addr)
{

}

uint8_t io_read(struct gb *gb, uint16_t addr)
{

}

uint8_t unused_read(struct gb *gb, uint16_t addr)
{

}

uint8_t get_bus_region(uint16_t addr)
{
    return (((addr <= 0x7fff) << 0) |
            ((addr >= 0x8000 && addr <= 0xfdff) << 1) |
            ((addr >= 0xff80 && addr <= 0xfffe) << 1) |
            ((addr >= 0xfe00 && addr <= 0xfe9f) << 2) |
            ((addr >= 0xfea0 && addr <= 0xfeff) << 4) |
            ((addr >= 0xff00 && addr <= 0xff7f) << 3) |
            ((addr == 0xffff) << 3));
}

void (*write_function[])(struct gb *gb, uint16_t addr, uint8_t val) = {
    [ROM] = rom_write,
    [RAM] = ram_write,
    [OAM] = oam_write,
    [IO] = io_write,
    [UNUSED] = unused_write,
};

uint8_t (*read_function[])(struct gb *gb, uint16_t addr) = {
    [ROM] = rom_read,
    [RAM] = ram_read,
    [OAM] = oam_read,
    [IO] = io_read,
    [UNUSED] = unused_read,
};

void bus_wait(struct gb *gb)
{
    sm83_cycle(gb);
    gb->record[gb->record_index].addr = 0;
    gb->record[gb->record_index].val = 0;
    gb->record[gb->record_index].mode = DUMMY;
    gb->record_index++;
}

uint8_t bus_read(struct gb *gb, uint16_t addr)
{
    sm83_cycle(gb);
    gb->record[gb->record_index].addr = addr;
    gb->record[gb->record_index].val = gb->mem[addr];
    gb->record[gb->record_index].mode = READ;
    gb->record_index++;
    return gb->mem[addr];
}

void bus_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    sm83_cycle(gb);
    gb->record[gb->record_index].addr = addr;
    gb->record[gb->record_index].val = val;
    gb->record[gb->record_index].mode = WRITE;
    gb->record_index++;
    gb->mem[addr] = val;
}