#include "cartridge.h"

static char *cart_types[] = {
	[0x00] = "ROM ONLY",
	[0x01] = "MBC1",
	[0x02] = "MBC1+RAM",
	[0x03] = "MBC1+RAM+BATTERY",
	[0x05] = "MBC2",
	[0x06] = "MBC2+BATTERY",
	[0x08] = "ROM+RAM",
	[0x09] = "ROM+RAM+BATTERY",
	[0x0B] = "MMM01",
	[0x0C] = "MMM01+RAM",
	[0x0D] = "MMM01+RAM+BATTERY",
	[0x0F] = "MBC3+TIMER+BATTERY",
	[0x10] = "MBC3+TIMER+RAM+BATTERY",
	[0x11] = "MBC3",
	[0x12] = "MBC3+RAM",
	[0x13] = "MBC3+RAM+BATTERY",
	[0x15] = "MBC4",
	[0x16] = "MBC4+RAM",
	[0x17] = "MBC4+RAM+BATTERY",
	[0x19] = "MBC5",
	[0x1A] = "MBC5+RAM",
	[0x1B] = "MBC5+RAM+BATTERY",
	[0x1C] = "MBC5+RUMBLE",
	[0x1D] = "MBC5+RUMBLE+RAM",
	[0x1E] = "MBC5+RUMBLE+RAM+BATTERY",
	[0xFC] = "POCKET CAMERA",
	[0xFD] = "BANDAI TAMA5",
	[0xFE] = "HuC3",
	[0xFF] = "HuC1+RAM+BATTERY",
};

static char *rom_size[] = {
    "32 KiB",
    "64 KiB",
    "128 KiB",
    "256 KiB",
    "512 KiB",
    "1 MiB",
    "2 MiB",
    "4 MiB",
    "8 MiB",
    "1.1 MiB",
    "1.2 MiB",
    "1.5 MiB",
};

static char *sram_size[] = {
    "0",
    "0",
    "8 KiB",
    "32 KiB",
    "128 KiB",
    "64 KiB",
};

static int sram_size_num[] = {
    0, 0, 8 * KiB, 32 * KiB, 128 * KiB, 64 * KiB
};

uint8_t (*read_func[])(struct gb *gb, uint16_t addr) = {
    [NO_MBC] = no_mbc_read,
    [MBC1] = mbc1_read,
    [MBC1_RAM] = mbc1_read,
    [MBC1_RAM_BATTERY] = mbc1_read,
};

void (*write_func[])(struct gb *gb, uint16_t addr, uint8_t val) = {
    [NO_MBC] = no_mbc_write,
    [MBC1] = mbc1_write,
    [MBC1_RAM] = mbc1_write,
    [MBC1_RAM_BATTERY] = mbc1_write,
};

void cartridge_load(struct gb *gb, char *cartridge_path)
{
    FILE *fp = NULL;
    long file_size;

    if (cartridge_path != NULL) {
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

void cartridge_get_infos(struct gb *gb)
{
    if (!gb->cart.cartridge_loaded) {
        fprintf(stderr, "Error: No cartridge found.\n");
        return;
    }
    for (uint16_t i = 0x0134, j = 0; i <= 0x0143; i++) {
        if (gb->cart.rom[i] != 0x00)
            gb->cart.infos.name[j++] = gb->cart.rom[i];
        else {
            gb->cart.infos.name[j] = '\0';
            break;
        }
    }
    gb->cart.infos.type = gb->cart.rom[0x0147];
    gb->cart.infos.rom_size = 32 * KiB * (1 << gb->cart.rom[0x0148]);
    gb->cart.infos.ram_size = sram_size_num[gb->cart.rom[0x0149]];
    gb->cart.infos.bank_size = gb->cart.infos.rom_size / (16 * KiB);
    
    // print cartridge infos
    printf("name: %s\n", gb->cart.infos.name);
    printf("mbc type: %s\n", cart_types[gb->cart.infos.type]);
    printf("ROM size: %s\n", rom_size[gb->cart.rom[0x0148]]);
    printf("RAM size: %s\n", sram_size[gb->cart.rom[0x0149]]);
    printf("bank size: %d\n", gb->cart.infos.bank_size);
}

void rom_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    write_func[gb->cart.infos.type](gb, addr, val);
}

uint8_t rom_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    ret = (gb->cart.infos.bank_size == 2) ? no_mbc_read(gb, addr) : read_func[gb->cart.infos.type](gb, addr);
    return ret;
}

void load_state_after_booting(struct gb *gb)
{
    // TODO: missing states of APU

    gb->mode = NORMAL;

    // cartridge

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

    // mbcs
    gb->mbc.mbc1.ram_enable = 0;
    gb->mbc.mbc1.banking_mode = 0;
    gb->mbc.mbc1.rom_bank_number = 0;
    gb->mbc.mbc1.ram_bank_number = 0;

}