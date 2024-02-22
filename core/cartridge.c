#include "cartridge.h"

void cartridge_load(struct gb *gb, char *cartridge_path, char *boot_rom_path)
{
    FILE *fp = NULL;
    long file_size;

    if (boot_rom_path != NULL) {
        fp = fopen(boot_rom_path, "r");
        if (!fp) {
            printf("The boot rom path is wrong\n");
            goto file_not_found;
        }
        if (fread(gb->cart.boot_code, 1, 0xff, fp) != 0xff)
            goto read_failed;
        gb->cart.boot_rom_loaded = true;
    }

    if (cartridge_path != NULL) {
        if (boot_rom_path != NULL)
            fclose(fp);
        fp = fopen(cartridge_path, "r");
        if (!fp) {
            printf("The cartridge path is wrong\n");
            goto file_not_found;
        }
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);
        if (fread(gb->cart.rom, 1, file_size, fp) != file_size)
            goto read_failed;
    }

file_not_found:
    fclose(fp);
    return;    
read_failed:
    fprintf(stderr, "Read failed\n");
    fclose(fp);
    exit(EXIT_FAILURE);
}


void rom_write(struct gb *gb, uint16_t addr, uint8_t val)
{

}

uint8_t rom_read(struct gb *gb, uint16_t addr)
{
    return (addr <= 0xff && gb->cart.boot_rom_loaded) 
            ? gb->cart.boot_code[addr] : gb->cart.rom[addr];
}

void load_state_after_booting(struct gb *gb)
{
    // TODO: missing states of PPU, APU

    gb->mode = NORMAL;
    // cartridge
    gb->cart.boot_rom_loaded = false;

    // cpu
    gb->cpu.regs.pc = 0x100;
    gb->cpu.regs.a = 0x01;
    gb->cpu.regs.flag_z = 1;
    gb->cpu.regs.flag_n = 0;
    gb->cpu.regs.bc = 0x0013;
    gb->cpu.regs.de = 0x00d8;
    gb->cpu.regs.hl = 0x014d;
    gb->cpu.regs.sp = 0xfffe; 

    // interrupt
    gb->intr.flag = 0xe1;
    gb->intr.ie = 0x00;

    // timer
    gb->tim.div = 0xab;
    gb->tim.tima = 0x00;
    gb->tim.tma = 0x00;
    gb->tim.tac.val = 0xf8;
    gb->tim.old_edge = 0;
}