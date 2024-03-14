#pragma once

typedef enum {
    DECREASE,
    INCREASE,
} volenv_dir_t;

#include "gb.h"

struct apu_channel;
struct gb;

/* Sound Channel 1 - Pulse */
#define APU_REG_NR10                0xff10          /* Sweep period, Negate, Shift */
#define APU_REG_NR11                0xff11          /* Duty, Length load */
#define APU_REG_NR12                0xff12          /* Starting volume, Envelope add mode, Period */
#define APU_REG_NR13                0xff13          /* Frequency LSB */
#define APU_REG_NR14                0xff14          /* Trigger, Length enable, Frequency MSB */

/* Sound Channel 2 - Pulse */
#define APU_REG_NR21                0xff16          /* Duty, Length load(64-L) */
#define APU_REG_NR22                0xff17          /* Starting volume, Envelope add mode, Period */
#define APU_REG_NR23                0xff18          /* Frequency LSB */
#define APU_REG_NR24                0xff19          /* Trigger, Length enable, Frequency MSB */

/* Wave Channel */
#define APU_REG_NR30                0xff1a          /* DAC power */
#define APU_REG_NR31                0xff1b          /* Length load (256-L) */
#define APU_REG_NR32                0xff1c          /* Volume code (00 = 0%, 01 = 100%, 10 = 50%, 11 = 25%) */
#define APU_REG_NR33                0xff1d          /* Frequency LSB */
#define APU_REG_NR34                0xff1e          /* Trigger, Length enable, Frequency MSB */

/* Noise Channel */
#define APU_REG_NR41                0xff20          /* Length load(64-L) */
#define APU_REG_NR42                0xff21          /* Starting volume, Envelope add mode, Period */
#define APU_REG_NR43                0xff22          /* Clock shift, Width mode of LFSR, Divisor code */
#define APU_REG_NR44                0xff23          /* Trigger, Length enable */

/* Control/Status */
#define APU_REG_NR50                0xff24          /* Vin L enable, Left vol, Vin R enable, Right vol */
#define APU_REG_NR51                0xff25          /* Left enables, Right enables */
#define APU_REG_NR52                0xff26          /* Power control/status, Channel length statuses */

void apu_regs_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t apu_regs_read(struct gb *gb, uint16_t addr);
void apu_ram_write(struct gb *gb, uint16_t addr, uint8_t val);
uint8_t apu_ram_read(struct gb *gb, uint16_t addr);
void apu_tick(struct gb *gb);

/* frequency sweep helpers */
uint8_t get_sweep_period(struct apu_channel *chan);
bool is_sweep_negate(struct apu_channel *chan);
uint8_t get_sweep_shift(struct apu_channel *chan);

/* square wave helpers */
uint8_t get_square_duty_cycle(struct apu_channel *chan);
bool is_length_counter_enable(struct apu_channel *chan);

/* envelope function helpers */
uint8_t get_envelope_volume(struct apu_channel *chan);
bool get_envelope_add_mode(struct apu_channel *chan);
uint8_t get_envelope_period(struct apu_channel *chan);

/* frequency helpers */
uint16_t get_frequency(struct apu_channel *chan);

/* noise channel helpers */
uint8_t get_noise_clock_shift(struct apu_channel *chan);
bool get_noise_width_mode(struct apu_channel *chan);
uint16_t get_noise_divisor(struct apu_channel *chan);

/* dac helpers */
bool is_dac_on(struct apu_channel *chan);

/* other helpers */
uint8_t get_length_load(struct apu_channel *chan);
uint8_t get_register_num(uint16_t addr);
struct apu_channel *get_channel_from_addr(struct gb *gb, uint16_t addr);
float get_channel_amplitude(struct apu_channel *chan);