#include "bus.h"

enum BUS_REGION {
    ROM = 1,
    RAM = 2,
    OAM = 4,
    IO  = 8,
    UNUSED = 16,
};

bool is_interrupt_reg(uint16_t addr)
{
    return (addr == INTR_REG_IE || addr == INTR_REG_IF);
}

bool is_timer_reg(uint16_t addr)
{
    return (addr == TIM_REG_DIV || addr == TIM_REG_TAC ||
            addr == TIM_REG_TIMA || addr == TIM_REG_TMA);
}

bool is_ppu_reg(uint16_t addr)
{
    return (addr == PPU_REG_LCDC || addr == PPU_REG_STAT || addr == PPU_REG_SCY ||
            addr == PPU_REG_SCX || addr == PPU_REG_LY || addr == PPU_REG_LYC ||
            addr == PPU_REG_BGP || addr == PPU_REG_OBP0 || addr == PPU_REG_OBP1 ||
            addr == PPU_REG_WY || addr == PPU_REG_WX);
}

/* write functions */
void ram_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    if (addr >= 0xe000 && addr <= 0xfdff)
        addr &= 0xddff;
    gb->mem[addr] = val;
}

void oam_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    gb->mem[addr] = val;
}

void io_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    // TODO: complete this function. Now just treat IO memory 
    //       range like an array 
    if (addr == 0xff50 && val == 1 && gb->cart.boot_rom_loaded)
        gb->cart.boot_rom_loaded = false;
    else if (is_interrupt_reg(addr))
        interrupt_write(gb, addr, val);
    else if (is_timer_reg(addr))
        timer_write(gb, addr, val);
    else if (is_ppu_reg(addr))
        ppu_write(gb, addr, val);
    else
        gb->mem[addr] = val;
}

void unused_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    gb->mem[addr] = val;
}

/* write functions */
uint8_t ram_read(struct gb *gb, uint16_t addr)
{
    if (addr >= 0xe000 && addr <= 0xfdff)
        addr &= 0xddff;
    return gb->mem[addr];
}

uint8_t oam_read(struct gb *gb, uint16_t addr)
{
    return gb->mem[addr];
}

uint8_t io_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    if (is_interrupt_reg(addr))
        ret = interrupt_read(gb, addr);
    else if (is_timer_reg(addr))
        ret = timer_read(gb, addr);
    else if (is_ppu_reg(addr))
        ret = ppu_read(gb, addr);
    return ret;
}

uint8_t unused_read(struct gb *gb, uint16_t addr)
{
    return gb->mem[addr];
}

uint8_t bus_get_mem_region(uint16_t addr)
{
    return (((addr <= 0x7fff) << 0) |
            ((addr >= 0x8000 && addr <= 0xdfff) << 1) |
            ((addr >= 0xe000 && addr <= 0xfdff) << 1) |
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
}

uint8_t bus_read(struct gb *gb, uint16_t addr)
{
    sm83_cycle(gb);
    return read_function[bus_get_mem_region(addr)](gb, addr);
}

void bus_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    sm83_cycle(gb);
    write_function[bus_get_mem_region(addr)](gb, addr, val);
}