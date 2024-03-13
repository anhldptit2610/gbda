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

void mbc1_ram_alloc(struct gb *gb)
{
    if (gb->cart.infos.ram_size > 0) {
        gb->mbc.mbc1.ram = (uint8_t *)malloc(sizeof(uint8_t) * gb->cart.infos.ram_size);
        if (!gb->mbc.mbc1.ram) {
            fprintf(stderr, "Can't allocate RAM for the system\n");
            exit(1);
        }
        printf("Cartridge RAM allocated\n");
    }
}

void mbc1_ram_dump(struct gb *gb)
{
    char *save_file;

    asprintf(&save_file, "%s.sav", gb->cart.infos.name);
    FILE *fp = fopen(save_file, "w");
    if (!fp) {
        fprintf(stderr, "Can't open/create save file\n");
        return;
    }
    if (fwrite(gb->mbc.mbc1.ram, 1, gb->cart.infos.ram_size, fp) != gb->cart.infos.ram_size)
        fprintf(stderr, "Write failed\n");
    fclose(fp);
}

void mbc1_ram_load(struct gb *gb)
{
    char *save_file;

    asprintf(&save_file, "%s.sav", gb->cart.infos.name);
    FILE *fp = fopen(save_file, "r");
    if (!fp) {
        printf("This ROM doesn't have save file available\n");
        return;
    }
    if (fread(gb->mbc.mbc1.ram, 1, gb->cart.infos.ram_size, fp) != gb->cart.infos.ram_size)
        fprintf(stderr, "RAM loading failed\n");
    fclose(fp); 
}

void mbc1_ram_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    gb->mbc.mbc1.ram[addr - 0xa000] = val;
}

uint8_t mbc1_ram_read(struct gb *gb, uint16_t addr)
{
    return gb->mbc.mbc1.ram[addr - 0xa000];
}

void mbc_init(struct gb *gb)
{
    switch (gb->cart.infos.type) {
    case MBC1_RAM:
    case MBC1_RAM_BATTERY:
        mbc1_ram_alloc(gb);
        mbc1_ram_load(gb);
        break;
    default:
        break;
    }
}