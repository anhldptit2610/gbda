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
        fread(gb->cart.boot_code, 1, 0xff, fp);
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
        fread(gb->cart.rom, 1, file_size, fp);
    }

file_not_found:
    fclose(fp);
    return;    
}


void rom_write(struct gb *gb, uint16_t addr, uint8_t val)
{

}

uint8_t rom_read(struct gb *gb, uint16_t addr)
{

}

void load_state_after_booting(struct gb *gb)
{
    // TODO: complete this function
}