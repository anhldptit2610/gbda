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
    [MBC3_RAM_BATTERY] = mbc3_read,
};

void (*write_func[])(struct gb *gb, uint16_t addr, uint8_t val) = {
    [NO_MBC] = no_mbc_write,
    [MBC1] = mbc1_write,
    [MBC1_RAM] = mbc1_write,
    [MBC1_RAM_BATTERY] = mbc1_write,
    [MBC3_RAM_BATTERY] = mbc3_write,
};

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
    if (gb->cart.infos.type == MBC1_RAM_BATTERY)
        gb->mbc.mbc1.has_battery = true;
    gb->cart.infos.rom_size = 32 * KiB * (1 << gb->cart.rom[0x0148]);
    gb->cart.infos.ram_size = sram_size_num[gb->cart.rom[0x0149]];
    gb->cart.infos.bank_size = gb->cart.infos.rom_size / (16 * KiB);
}    

void cartridge_print_info(struct gb *gb)
{
    // print cartridge infos
    printf("name: %s\n", gb->cart.infos.name);
    printf("mbc type: %s\n", cart_types[gb->cart.infos.type]);
    printf("ROM size: %s\n", rom_size[gb->cart.rom[0x0148]]);
    printf("RAM size: %s\n", sram_size[gb->cart.rom[0x0149]]);
    printf("bank size: %d\n", gb->cart.infos.bank_size);
}

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
        gb->cart.cartridge_loaded = true;
        cartridge_get_infos(gb);
        cartridge_print_info(gb);
        mbc_init(gb);
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
    write_func[gb->cart.infos.type](gb, addr, val);
}

uint8_t rom_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    ret = read_func[gb->cart.infos.type](gb, addr);
    return ret;
}

void load_state_after_booting(struct gb *gb)
{
    // make code shorter, easier to read
    struct sm83 *cpu = &gb->cpu;
    struct ppu *ppu = &gb->ppu;
    struct interrupt *interrupt = &gb->interrupt;
    struct timer *timer = &gb->timer;
    struct dma *dma = &gb->dma;
    struct joypad *joypad = &gb->joypad;
    struct mbc *mbc = &gb->mbc;
    struct apu *apu = &gb->apu;

    gb->mode = NORMAL;
    if (!gb->volume_set)
        gb->user_volume = 7;

    // cpu
    cpu->pc = 0x100;
    cpu->af.a = 0x01;
    cpu->af.flag.z = 1;
    cpu->af.flag.n = 0;
    cpu->bc.val = 0x0013;
    cpu->de.val = 0x00d8;
    cpu->hl.val = 0x014d;
    cpu->sp = 0xfffe; 

    // interrupt
    interrupt->flag = 0xe1;
    interrupt->ie = 0x00;

    // timer
    timer->div = 0xab;
    timer->tima = 0x00;
    timer->tma = 0x00;
    timer->tac.val = 0xf8;
    timer->old_edge = 0;

    // ppu
    ppu->lcdc.val = 0x91;
    ppu->stat.val = 0x85;
    ppu->scy = 0x00;
    ppu->scx = 0x00;
    ppu->ly = 0x00;
    ppu->lyc = 0x00;
    ppu->bgp = 0xfc;
    ppu->wy = 0x00;
    ppu->wx = 0x00;
    ppu->ticks = 0;
    ppu->mode = OAM_SCAN;
    memset(ppu->frame_buffer, COLOR_WHITE, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    ppu->frame_ready = false;
    ppu->oam_entry_cnt = 0;
    ppu->sprite_cnt = 0;
    ppu->stat_intr_line = false;
    ppu->stat_intr_src.val = 0;
    ppu->window_in_frame = false;
    ppu->window_line_cnt = 0;
    ppu->draw_window_this_line = false;

    // dma
    dma->mode = OFF;
    dma->reg = 0xff;

    // joypad
    joypad->a = 1;
    joypad->b = 1;
    joypad->select = 1;
    joypad->start = 1;
    joypad->up = 1;
    joypad->down = 1;
    joypad->left = 1;
    joypad->right = 1;
    joypad->joyp.val = 0xcf;

    // mbc1
    mbc->mbc1.ram_enable = 0;
    mbc->mbc1.banking_mode = 0;
    mbc->mbc1.rom_bank = 0;
    mbc->mbc1.ram_bank = 0;
    mbc->mbc1.has_battery = false;

    // mbc3
    mbc->mbc3.ram_enable = false;
    mbc->mbc3.rom_bank = 0;
    mbc->mbc3.ram_bank = 0;



    // TODO: update each channel after booting status 
    //       when implementing them. 

    apu->tick = 0;
    apu->frame_sequencer = 0;

    // apu square1
    apu->sqr1.name = SQUARE1;
    apu->sqr1.is_active = true;
    apu->sqr1.regs.nrx0 = 0x80;
    apu->sqr1.regs.nrx1 = 0xbf;
    apu->sqr1.regs.nrx2 = 0xf3;
    apu->sqr1.regs.nrx3 = 0xff;
    apu->sqr1.regs.nrx4 = 0xbf;
    apu->sqr1.is_dac_on = is_dac_on(&gb->apu.sqr1);
    apu->sqr1.length.counter = get_length_load(&gb->apu.sqr1);
    apu->sqr1.timer = get_frequency(&gb->apu.sqr1);
    apu->sqr1.left_output = true;
    apu->sqr1.right_output = true;
    apu->sqr1.volume_envelope.add_mode = get_envelope_add_mode(&gb->apu.sqr1);
    apu->sqr1.volume = 15;
    apu->sqr1.pos = 0;
    apu->sqr1.volume_envelope.period = get_envelope_period(&gb->apu.sqr1);
    apu->sqr1.frequency_sweep.period = get_sweep_period(&gb->apu.sqr1);
    apu->sqr1.frequency_sweep.negate = BIT(gb->apu.sqr1.regs.nrx0, 3);
    apu->sqr1.frequency_sweep.shift = get_sweep_shift(&gb->apu.sqr1);
    apu->sqr1.frequency_sweep.is_active = true;
    apu->sqr1.frequency_sweep.timer = gb->apu.sqr1.frequency_sweep.period;
    apu->sqr1.frequency_sweep.shadow_frequency = get_frequency(&gb->apu.sqr1);

    // apu square2
    apu->sqr2.name = SQUARE2;
    apu->sqr2.is_active = false;
    apu->sqr2.regs.nrx1 = 0x3f;
    apu->sqr2.regs.nrx2 = 0x00;
    apu->sqr2.regs.nrx3 = 0xff;
    apu->sqr2.regs.nrx4 = 0xbf;
    apu->sqr2.is_dac_on = is_dac_on(&gb->apu.sqr2);
    apu->sqr2.length.counter = 0;
    apu->sqr2.timer = 0;
    apu->sqr2.left_output = true;
    apu->sqr2.right_output = true;
    apu->sqr2.volume_envelope.add_mode = get_envelope_add_mode(&gb->apu.sqr2);
    apu->sqr2.volume = 0;
    apu->sqr2.pos = 0;
    apu->sqr2.volume_envelope.period = get_envelope_period(&gb->apu.sqr2);

    // apu wave
    apu->wave.name = WAVE;
    apu->wave.is_active = false;
    apu->wave.regs.nrx0 = 0x7f;
    apu->wave.regs.nrx1 = 0xff;
    apu->wave.regs.nrx2 = 0x9f;
    apu->wave.regs.nrx3 = 0xff;
    apu->wave.regs.nrx4 = 0xbf;
    apu->wave.is_dac_on = BIT(gb->apu.wave.regs.nrx0, 7);
    apu->wave.left_output = true;
    apu->wave.right_output = false;
    apu->wave.pos = 0;
    apu->wave.is_dac_on = is_dac_on(&apu->wave);

    // apu noise
    apu->noise.name = NOISE;
    apu->noise.is_active = false;
    apu->noise.regs.nrx1 = 0xff;
    apu->noise.regs.nrx2 = 0x00;
    apu->noise.regs.nrx3 = 0x00;
    apu->noise.regs.nrx4 = 0xbf;
    apu->noise.left_output = true;
    apu->noise.right_output = false;
    apu->noise.is_dac_on = is_dac_on(&gb->apu.noise);
    apu->noise.lfsr.clock_shift = get_noise_clock_shift(&gb->apu.noise);
    apu->noise.lfsr.divisor = get_noise_divisor(&gb->apu.noise);
    apu->noise.lfsr.width_mode = get_noise_width_mode(&gb->apu.noise);
    apu->noise.timer = apu->noise.lfsr.divisor << apu->noise.lfsr.clock_shift;
    apu->noise.volume = get_envelope_volume(&gb->apu.noise);
    apu->noise.volume_envelope.add_mode = get_envelope_add_mode(&gb->apu.noise);
    apu->noise.volume_envelope.period = get_envelope_period(&gb->apu.noise);
    apu->noise.lfsr.reg = 0x7fff;

    // apu global control
    apu->ctrl.name = CTRL;
    apu->ctrl.regs.nrx0 = 0x77;
    apu->ctrl.regs.nrx1 = 0xf3;
    apu->ctrl.regs.nrx2 = 0xf1;

    // sound panning
    apu->noise.left_output = true;
    apu->wave.left_output = true;
    apu->sqr2.left_output = true;
    apu->sqr1.left_output = true;
    apu->noise.right_output = false;
    apu->wave.right_output = false;
    apu->sqr2.right_output = true;
    apu->sqr1.right_output = true;

    apu->sample_buffer.ptr = 0;
    memset(apu->sample_buffer.buf, 0, BUFFER_SIZE * sizeof(int16_t));
    apu->sample_buffer.is_full = false;
}