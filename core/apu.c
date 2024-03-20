#include "apu.h"

uint8_t nrxx_or_val[6][5] = {
    [0] = {0x00, 0x00, 0x00, 0x00, 0x00},       // don't use this
    [1] = {0x80, 0x3f, 0x00, 0xff, 0xbf},
    [2] = {0xff, 0x3f, 0x00, 0xff, 0xbf},
    [3] = {0x7f, 0xff, 0x9f, 0xff, 0xbf},
    [4] = {0xff, 0xff, 0x00, 0x00, 0xbf},
    [5] = {0x00, 0x00, 0x70, 0x00, 0x00},
};

uint32_t noise_divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};

uint8_t square_wave[4][8] = {
    [0] = {0, 0, 0, 0, 0, 0, 0, 1},       /* 12.5% duty cycle */
    [1] = {1, 0, 0, 0, 0, 0, 0, 1},       /* 25% duty cycle */
    [2] = {1, 0, 0, 0, 0, 1, 1, 1},       /* 50% duty cycle */
    [3] = {0, 1, 1, 1, 1, 1, 1, 0},       /* 75% duty cycle */
};

uint8_t wave_channel_shift[4] = {4, 0, 1, 2};

/**********************************************************/
/*****************  Supporting functions ******************/
/**********************************************************/

/* frequency sweep helpers */
uint8_t get_sweep_period(struct apu_channel *chan)
{
    return (chan->regs.nrx0 >> 4) & 0x07;
}

bool is_sweep_negate(struct apu_channel *chan)
{
    return BIT(chan->regs.nrx0, 3);
}

uint8_t get_sweep_shift(struct apu_channel *chan)
{
    return (chan->regs.nrx0 & 0x07);
}

/* square wave helpers */
uint8_t get_square_duty_cycle(struct apu_channel *chan)
{
    return (chan->regs.nrx1 >> 6) & 0x03;
}


bool is_length_counter_enable(struct apu_channel *chan)
{
    return BIT(chan->regs.nrx4, 6);
}

/* envelope function helpers */
uint8_t get_envelope_volume(struct apu_channel *chan)
{
    return (chan->regs.nrx2 >> 4) & 0x0f;
}

bool get_envelope_add_mode(struct apu_channel *chan)
{
    return BIT(chan->regs.nrx2, 3);
}

uint8_t get_envelope_period(struct apu_channel *chan)
{
    return chan->regs.nrx2 & 0x07;
}

/* frequency helpers */
uint16_t get_frequency(struct apu_channel *chan)
{
    return TO_U16(chan->regs.nrx3, chan->regs.nrx4 & 0x07);
}

/* dac helpers */
bool is_dac_on(struct apu_channel *chan)
{
    return (chan->name == WAVE) ? BIT(chan->regs.nrx0, 7) : (chan->regs.nrx2 & 0xf8);
}

/* noise channel helpers */

bool lfsr_tick(struct apu_channel *chan)
{
    uint16_t lfsr = chan->lfsr.reg, feedback = BIT(lfsr, 0) ^ BIT(lfsr, 1);

    lfsr = ((lfsr >> 1) & ~(1U << 14)) | (feedback << 14);
    if (chan->lfsr.width_mode)
        lfsr = (lfsr & ~(1U << 6)) | (feedback << 6);
    chan->lfsr.reg = lfsr & 0x7fff;
    return BIT(chan->lfsr.reg, 0);
}

uint8_t get_noise_clock_shift(struct apu_channel *chan)
{
    return (chan->regs.nrx3 >> 4) & 0x0f;
}

bool get_noise_width_mode(struct apu_channel *chan)
{
    return BIT(chan->regs.nrx3, 3);
}

uint16_t get_noise_divisor(struct apu_channel *chan)
{
    return noise_divisor[chan->regs.nrx3 & 0x07];
}

/* other helpers */
uint8_t get_wave_channel_sample(struct gb *gb)
{
    uint8_t pos = gb->apu.wave.pos;

    return (pos % 2 == 0) ? (gb->apu.wave_ram[pos / 2] >> 4) & 0x0f : gb->apu.wave_ram[pos / 2] & 0x0f;
}

uint8_t get_register_num(uint16_t addr)
{
    int ret = 0;

    switch (addr) {
    case APU_REG_NR10:
    case APU_REG_NR30:
    case APU_REG_NR50:
        ret = 0;
        break;
    case APU_REG_NR11:
    case APU_REG_NR21:
    case APU_REG_NR31:
    case APU_REG_NR41:
    case APU_REG_NR51:
        ret = 1;
        break;
    case APU_REG_NR12:
    case APU_REG_NR22:
    case APU_REG_NR32:
    case APU_REG_NR42:
    case APU_REG_NR52:
        ret = 2;
        break;
    case APU_REG_NR13:
    case APU_REG_NR23:
    case APU_REG_NR33:
    case APU_REG_NR43:
        ret = 3;
        break;
    case APU_REG_NR14:
    case APU_REG_NR24:
    case APU_REG_NR34:
    case APU_REG_NR44:
        ret = 4;
        break;
    default:
        break;
    }
    return ret;
}

void load_new_frequency(struct apu_channel *chan, uint16_t frequency)
{
    chan->regs.nrx3 = LSB(frequency);
    chan->regs.nrx4 = (chan->regs.nrx4 & 0xf8) | (MSB(frequency) & 0x07);
}

uint8_t get_length_load(struct apu_channel *chan)
{
    return (chan->name == WAVE) ? chan->regs.nrx1 : (chan->regs.nrx1 & 0x3f);
}

struct apu_channel *get_channel_from_addr(struct gb *gb, uint16_t addr)
{
    struct apu_channel *ret = NULL;

    if (addr == APU_REG_NR10 || addr == APU_REG_NR11 || addr == APU_REG_NR12 ||
        addr == APU_REG_NR13 || addr == APU_REG_NR14)
        ret = &gb->apu.sqr1;
    else if (addr == APU_REG_NR21 || addr == APU_REG_NR22 || addr == APU_REG_NR23 ||
        addr == APU_REG_NR24)
        ret = &gb->apu.sqr2;
    else if (addr == APU_REG_NR30 || addr == APU_REG_NR31 || addr == APU_REG_NR32 ||
        addr == APU_REG_NR33 || addr == APU_REG_NR34)
        ret = &gb->apu.wave;
    else if (addr == APU_REG_NR41 || addr == APU_REG_NR42 || addr == APU_REG_NR43 ||
        addr == APU_REG_NR44)
        ret = &gb->apu.noise;
    else if (addr == APU_REG_NR50 || addr == APU_REG_NR51 || addr == APU_REG_NR52)
        ret = &gb->apu.ctrl;
    return ret;
}

float get_channel_amplitude(struct apu_channel *chan)
{
    int dac_input = (chan->name != WAVE) ? chan->output * chan->volume : chan->output;
    float dac_output = (is_dac_on(chan) && chan->is_active) ? (dac_input / 7.5) - 1.0 : 0.0;

    return dac_output;
}

/* This function performs frequency calculation and overflow check */
uint16_t calculate_frequency(struct apu_channel *chan)
{
    uint16_t new_frequency;
    uint8_t shift_amount;

    shift_amount = get_sweep_shift(chan);
    new_frequency = chan->frequency_sweep.shadow_frequency + ((chan->frequency_sweep.negate)
            ? (chan->frequency_sweep.shadow_frequency >> shift_amount) :  -1 * (chan->frequency_sweep.shadow_frequency >> shift_amount));
    if (new_frequency > 2047)
        chan->is_active = false; 
    return new_frequency;
}

/***************************************************************************************/

void trigger_channel(struct apu_channel *chan)
{
    chan->is_active = true;
    if (!chan->length.counter)
        chan->length.counter = (chan->name == WAVE) ? 256 : 64;
    if (chan->name == WAVE)
        chan->timer = (2048 - TO_U16(chan->regs.nrx3, chan->regs.nrx4 & 0x07)) * 2;
    else if (chan->name == NOISE)
        chan->timer = chan->lfsr.divisor << chan->lfsr.clock_shift;
    else
        chan->timer = (2048 - TO_U16(chan->regs.nrx3, chan->regs.nrx4 & 0x07)) * 4;
    chan->volume = get_envelope_volume(chan);
    chan->volume_envelope.period = get_envelope_period(chan);
    if (chan->name == NOISE) {
        chan->lfsr.reg = 0x7fff;
    } else if (chan->name == WAVE) {
        chan->pos = 0; 
    } else if (chan->name == SQUARE1) {
        chan->frequency_sweep.shadow_frequency = get_frequency(chan);
        chan->frequency_sweep.timer = get_sweep_period(chan);
        // the internal flag is set if either the sweep period or
        // shift are non-zero, cleared otherwise.
        chan->frequency_sweep.is_active = (chan->frequency_sweep.period != 0 || chan->frequency_sweep.shift != 0)
            ? true : false;
        if (chan->frequency_sweep.shift != 0)
            calculate_frequency(chan);
    }

    // if the channel's DAC is off, the channel will be disabled
    if (!is_dac_on(chan))
        chan->is_active = false;
}

void generate_sample(struct gb *gb)
{
    struct apu_channel *ch1 = &gb->apu.sqr1;
    struct apu_channel *ch2 = &gb->apu.sqr2;
    struct apu_channel *ch3 = &gb->apu.wave;
    struct apu_channel *ch4 = &gb->apu.noise;
    float left_mixer_output = 0.0;
    float right_mixer_output = 0.0;

    left_mixer_output = (((ch2->left_output) ? get_channel_amplitude(ch2) : 0.0) + 
                        ((ch1->left_output) ? get_channel_amplitude(ch1) : 0.0) +
                        ((ch3->left_output) ? get_channel_amplitude(ch3) : 0.0) + 
                         ((ch4->left_output) ? get_channel_amplitude(ch4) : 0.0)) / 4.0;
    left_mixer_output = left_mixer_output * (float)gb->apu.master_volume_right / 7.0;
    right_mixer_output = (((ch2->right_output) ? get_channel_amplitude(ch2) : 0.0) + 
                         ((ch1->right_output) ? get_channel_amplitude(ch1) : 0.0) + 
                         ((ch3->right_output) ? get_channel_amplitude(ch3) : 0.0) + 
                          ((ch4->right_output) ? get_channel_amplitude(ch4) : 0.0)) / 4.0;
    right_mixer_output = right_mixer_output * (float)gb->apu.master_volume_right / 7.0;
    gb->apu.sample_buffer.buf[gb->apu.sample_buffer.ptr] = (int16_t)(left_mixer_output * 32767.0f);
    gb->apu.sample_buffer.buf[gb->apu.sample_buffer.ptr + 1] = (int16_t)(right_mixer_output * 32767.0f);
    gb->apu.sample_buffer.ptr += 2;
    if (gb->apu.sample_buffer.ptr == 1024) {
        gb->apu.sample_buffer.is_full = true;
        gb->apu.sample_buffer.ptr = 0;
    }
}


void length_counter_tick(struct apu_channel *chan)
{
    if (!is_length_counter_enable(chan))
        return;
    chan->length.counter--;
    if (!chan->length.counter)
        chan->is_active = false;
}

void volume_envelope_tick(struct apu_channel *chan)
{
    uint8_t new_volume;

    if (chan->name == WAVE)
        return;
    if (!get_envelope_period(chan) || !chan->volume_envelope.period)
        return;
    chan->volume_envelope.period--;
    if (!chan->volume_envelope.period) {
        chan->volume_envelope.period = get_envelope_period(chan);
        new_volume = (get_envelope_add_mode(chan)) ? chan->volume + 1 : chan->volume - 1;
        if (IN_RANGE(new_volume, 0, 15))
            chan->volume = new_volume;
    }
}



void frequency_sweep_tick(struct apu_channel *chan)
{
    uint16_t new_frequency = 0;

    if (chan->frequency_sweep.timer > 0)
        chan->frequency_sweep.timer--;
    if (!chan->frequency_sweep.timer) {
        chan->frequency_sweep.timer = (!chan->frequency_sweep.period) ? 8 : chan->frequency_sweep.period;
        if (chan->frequency_sweep.is_active && chan->frequency_sweep.period) {
            // frequency calculation
            new_frequency = calculate_frequency(chan);
            if (new_frequency <= 2047 && chan->frequency_sweep.shift) {
                chan->frequency_sweep.shadow_frequency = new_frequency;
                load_new_frequency(chan, new_frequency);
                calculate_frequency(chan);
            }
        }
    }
}

void frame_sequencer_tick(struct apu *apu)
{
    struct apu_channel *square1 = &apu->sqr1;
    struct apu_channel *square2 = &apu->sqr2;
    struct apu_channel *wave = &apu->wave;
    struct apu_channel *noise = &apu->noise;

    switch (apu->frame_sequencer) {
    case 0:
        length_counter_tick(square1);
        length_counter_tick(square2);
        length_counter_tick(wave);
        length_counter_tick(noise);
        break;
    case 1:
        break;
    case 2:
        length_counter_tick(square1);
        length_counter_tick(square2);
        length_counter_tick(wave);
        length_counter_tick(noise);
        frequency_sweep_tick(square1);
        break;
    case 3:
        break;
    case 4:
        length_counter_tick(square1);
        length_counter_tick(square2);
        length_counter_tick(wave);
        length_counter_tick(noise);
        break;
    case 5:
        break;
    case 6:
        length_counter_tick(square1);
        length_counter_tick(square2);
        length_counter_tick(wave);
        length_counter_tick(noise);
        frequency_sweep_tick(square1);
        break;
    case 7:
        volume_envelope_tick(square1);
        volume_envelope_tick(square2);
        volume_envelope_tick(noise);
        break;
    default:
        break;
    }
    apu->frame_sequencer = (apu->frame_sequencer + 1) % 8;
}

void apu_regs_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    struct apu_channel *chan = get_channel_from_addr(gb, addr);
    int reg_num = get_register_num(addr);

    switch (reg_num) {
    case 0:
        chan->regs.nrx0 = val;
        if (chan->name == CTRL) {
            gb->apu.master_volume_left = (val >> 4) & 0x07;
            gb->apu.master_volume_right = val & 0x07;
        } else if (chan->name == SQUARE1) {
            chan->frequency_sweep.period = get_sweep_period(chan);
            chan->frequency_sweep.negate = BIT(val, 3);
            chan->frequency_sweep.shift = get_sweep_shift(chan);
        } else if (chan->name == WAVE) {
            chan->is_dac_on = BIT(val, 7);
        }
        break;
    case 1:
        chan->regs.nrx1 = val;
        if (chan->name == SQUARE1 || chan->name == SQUARE2) {
            chan->duty_cycle = val >> 6;
            chan->length.counter = val & 0x3f;
        } else if (chan->name == WAVE) {
            chan->length.counter = val;
        } else if (chan->name == NOISE) {
            chan->length.counter = val & 0x3f;
        } else if (chan->name == CTRL) {
            gb->apu.noise.left_output = BIT(val, 7);
            gb->apu.wave.left_output = BIT(val, 6);
            gb->apu.sqr2.left_output = BIT(val, 5);
            gb->apu.sqr1.left_output = BIT(val, 4);
            gb->apu.noise.right_output = BIT(val, 3);
            gb->apu.wave.right_output = BIT(val, 2);
            gb->apu.sqr2.right_output = BIT(val, 1);
            gb->apu.sqr1.right_output = BIT(val, 0);
        }
        break;
    case 2:
        chan->regs.nrx2 = val;
        if (chan->name == CTRL)
            gb->apu.is_active = BIT(val, 7);
        else if (chan->name == WAVE)
           chan->volume_code = (val >> 5) & 0x03; 
        break;
    case 3:
        chan->regs.nrx3 = val;
        if (chan->name == NOISE) {
            chan->lfsr.clock_shift = get_noise_clock_shift(chan);
            chan->lfsr.width_mode = get_noise_width_mode(chan);
            chan->lfsr.divisor = get_noise_divisor(chan);
        }
        break;
    case 4:
        chan->regs.nrx4 = val;
        if (BIT(val, 7))
            trigger_channel(chan);
        break;
    default:
        break;
    }
}

void apu_ram_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    gb->apu.wave_ram[addr - 0xff30] = val;
}

uint8_t apu_regs_read(struct gb *gb, uint16_t addr)
{
    struct apu_channel *chan = get_channel_from_addr(gb, addr);
    int reg_num = get_register_num(addr);
    uint8_t ret = 0xff;

    switch (reg_num) {
    case 0:
        ret = chan->regs.nrx0 | nrxx_or_val[chan->name][0];
        break;
    case 1:
        ret = chan->regs.nrx1 | nrxx_or_val[chan->name][1];
        break;
    case 2:
        ret = chan->regs.nrx2 | nrxx_or_val[chan->name][2];
        if (chan->name == CTRL) {
            ret |= ((uint8_t)gb->apu.sqr1.is_active << 0) |
                    ((uint8_t)gb->apu.sqr2.is_active << 1) |
                    ((uint8_t)gb->apu.wave.is_active << 2) |
                    ((uint8_t)gb->apu.noise.is_active << 3);
        }
        break;
    case 3:
        ret = chan->regs.nrx3 | nrxx_or_val[chan->name][3];
        break;
    case 4:
        ret = chan->regs.nrx4 | nrxx_or_val[chan->name][4];
        break;
    default:
        break;
    }
    return ret;
}

uint8_t apu_ram_read(struct gb *gb, uint16_t addr)
{
    return gb->apu.wave_ram[addr - 0xff30];
}

void ch1_tick(struct gb *gb)
{
    struct apu_channel *chan = &gb->apu.sqr1;

    if (!chan->is_active || !is_dac_on(chan))
        return;
    if (chan->timer > 0) {
        chan->timer--;
        if (!chan->timer) {
            chan->timer = (2048 - get_frequency(chan)) * 4;
            chan->output = square_wave[get_square_duty_cycle(chan)][chan->pos];
            chan->pos = (chan->pos == 7) ? 0 : chan->pos + 1;
        }
    }
}

void ch2_tick(struct gb *gb)
{
    struct apu_channel *chan = &gb->apu.sqr2;

    if (!chan->is_active || !is_dac_on(chan))
        return;
    if (chan->timer > 0) {
        chan->timer--;
        if (!chan->timer) {
            chan->timer = (2048 - get_frequency(chan)) * 4;
            chan->output = square_wave[get_square_duty_cycle(chan)][chan->pos];
            chan->pos = (chan->pos == 7) ? 0 : chan->pos + 1;
        }
    }
}

void ch3_tick(struct gb *gb)
{
    struct apu_channel *chan = &gb->apu.wave;

    if (!chan->is_active || !is_dac_on(chan))
        return;
    if (chan->timer > 0) {
        chan->timer--;
        if (!chan->timer) {
            chan->timer = (2048 - get_frequency(chan)) * 2;
            chan->output = get_wave_channel_sample(gb) >> wave_channel_shift[chan->volume_code];
            chan->pos = (chan->pos == 31) ? 0 : chan->pos + 1;
        }
    }
}

void ch4_tick(struct gb *gb)
{
    struct apu_channel *chan = &gb->apu.noise;

    if (!chan->is_active || !is_dac_on(chan))
        return;
    if (chan->timer > 0) {
        chan->timer--;
        if (!chan->timer) {
            chan->timer = chan->lfsr.divisor << chan->lfsr.clock_shift;
            chan->output = !lfsr_tick(chan);
        }
    }
}

void apu_tick(struct gb *gb)
{
    if (!BIT(gb->apu.ctrl.regs.nrx2, 7))
        return;
    gb->apu.tick++;
    ch1_tick(gb);
    ch2_tick(gb);
    ch3_tick(gb);
    ch4_tick(gb);

    if (!(gb->apu.tick % 8192)) {
        frame_sequencer_tick(&gb->apu);
        gb->apu.tick = 0;
    }

    /* Every 95 Hz, we get a sample. This would happen until the sampling
        buffer is full of 512 samples. 95 here is from the Game Boy's working
        frequency / 44100. */
    if (!(gb->apu.tick % 95))
        generate_sample(gb);
}