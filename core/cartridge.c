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
        gb->ppu.mode = OAM_SCAN;
        gb->ppu.lcdc.val = 0x00;
        gb->ppu.stat.val = 0x00;
        gb->ppu.scy = 0x00;
        gb->ppu.scx = 0x00;
        gb->ppu.ly = 0x00;
        gb->ppu.lyc = 0x00;
        gb->ppu.bgp = 0x00;
        gb->ppu.wy = 0x00;
        gb->ppu.wx = 0x00;
        gb->ppu.ticks = 0;
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
        printf("cartridge loaded\n");
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
    uint8_t ret = 0xff;

    if (addr <= 0xff && gb->cart.boot_rom_loaded)
        ret = gb->cart.boot_code[addr];
    else if (gb->cart.cartridge_loaded)
        ret = gb->cart.rom[addr];
    return ret;
}

void load_state_after_booting(struct gb *gb)
{
    // TODO: missing states of APU

    gb->mode = NORMAL;

    // cartridge
    gb->cart.boot_rom_loaded = false;

    // cpu
    gb->cpu.pc = 0x100;
    gb->cpu.af.a = 0x01;
    gb->cpu.af.flag.z = 1;
    gb->cpu.af.flag.n = 0;
    gb->cpu.bc.val = 0x0013;
    gb->cpu.de.val = 0x00d8;
    gb->cpu.hl.val = 0x014d;
    gb->cpu.sp = 0xfffe; 

    // interrupt
    gb->intr.flag = 0xe1;
    gb->intr.ie = 0x00;

    // timer
    gb->tim.div = 0xab;
    gb->tim.tima = 0x00;
    gb->tim.tma = 0x00;
    gb->tim.tac.val = 0xf8;
    gb->tim.old_edge = 0;

    // ppu
    gb->ppu.lcdc.val = 0x91;
    gb->ppu.stat.val = 0x85;
    gb->ppu.scy = 0x00;
    gb->ppu.scx = 0x00;
    gb->ppu.ly = 0x00;
    gb->ppu.lyc = 0x00;
    gb->ppu.bgp = 0xfc;
    gb->ppu.wy = 0x00;
    gb->ppu.wx = 0x00;
    gb->ppu.ticks = 0;
    gb->ppu.mode = OAM_SCAN;
    memset(gb->ppu.frame_buffer, COLOR_WHITE, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    gb->ppu.frame_ready = false;
    gb->ppu.oam_entry_cnt = 0;
    gb->ppu.sprite_cnt = 0;
    gb->ppu.stat_intr_line = false;
    gb->ppu.stat_intr_src.val = 0;
    gb->ppu.window_in_frame = false;
    gb->ppu.window_line_cnt = 0;
    gb->ppu.draw_window_this_line = false;

    // dma
    gb->dma.mode = OFF;
    gb->dma.reg = 0xff;

    // joypad
    gb->joypad.a = 1;
    gb->joypad.b = 1;
    gb->joypad.select = 1;
    gb->joypad.start = 1;
    gb->joypad.up = 1;
    gb->joypad.down = 1;
    gb->joypad.left = 1;
    gb->joypad.right = 1;
    gb->joypad.joyp.val = 0xcf;
}