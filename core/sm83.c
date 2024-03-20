#include "sm83.h"

int instr_cycle[] = {
//  x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, // 0x
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1, // 1x
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1, // 2x
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, // 3x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 4x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 5x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 6x
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 7x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 8x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 9x
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // Ax
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // Bx
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4, // Cx
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 1, 3, 6, 2, 4, // Dx
    3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4, // Ex
    3, 3, 2, 1, 0, 4, 2, 4, 4, 2, 4, 1, 0, 0, 2, 4, // Fx
};

int cb_instr_cycle[] = {
//  x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 1x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 2x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 3x 
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 4x 
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 5x 
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 6x 
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 7x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 8x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 9x 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Ax 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Bx 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Cx 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Dx 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Ex 
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // Fx 
};

void sm83_tick(struct gb *gb)
{
    for (int i = 0; i < 4; i++) {
        timer_tick(gb);
        ppu_tick(gb);
        apu_tick(gb);
    }
}

void sm83_cycle(struct gb *gb, int cycles)
{
    static int i = 0, passed_cycles = 0;

    for (int j = 0; j < cycles; j++) {
        passed_cycles += 1;
        if (passed_cycles == 95) {
            passed_cycles = 0;
        }
        sm83_tick(gb);

        // deal with DMA
        if (gb->dma.mode == WAITING) {
            gb->dma.mode = TRANSFERING;
        } else if (gb->dma.mode == TRANSFERING) {
            gb->oam[i] = dma_get_data(gb, gb->dma.start_addr + i);
            if (i == 0x9f) {
                gb->dma.mode = OFF;
                i = 0;
            } else {
                i++;
            }
        }
    }
}

void sm83_init(struct gb *gb)
{
    gb->cpu.pc = 0;
    gb->cart.cartridge_loaded = false;
}

uint8_t sm83_fetch_byte(struct gb *gb)
{
    uint8_t ret = 0xff;

    if (gb->mode == HALT) {
        ret = 0x76;
    } else {
        ret = bus_read(gb, gb->cpu.pc++);
    }
    return ret;
}

uint16_t sm83_fetch_word(struct gb *gb)
{
    uint8_t lsb = sm83_fetch_byte(gb);
    uint8_t msb = sm83_fetch_byte(gb);

    return TO_U16(lsb, msb);
}

void sm83_push_byte(struct gb *gb, uint8_t val)
{
    bus_write(gb, --gb->cpu.sp, val);
}

void sm83_push_word(struct gb *gb, uint16_t val)
{
    sm83_push_byte(gb, MSB(val));
    sm83_push_byte(gb, LSB(val));
}

uint8_t sm83_pop_byte(struct gb *gb)
{
    gb->cpu.sp++;
    return bus_read(gb, gb->cpu.sp - 1);
}

uint16_t sm83_pop_word(struct gb *gb)
{
    uint8_t lsb = sm83_pop_byte(gb);
    uint8_t msb = sm83_pop_byte(gb);

    return TO_U16(lsb, msb);
}

static inline void toggle_znh(struct gb *gb, uint8_t res, bool n, bool h)
{
    gb->cpu.af.flag.z = !res;
    gb->cpu.af.flag.n = n;
    gb->cpu.af.flag.h = h;
}

/* Instructions */

static inline void ld_indirect_hl_n(struct gb *gb, uint8_t n)
{
    bus_write(gb, gb->cpu.hl.val, n);
}

static inline void ldh_indirect_c_a(struct gb *gb)
{
    bus_write(gb, 0xff00 + gb->cpu.bc.c, gb->cpu.af.a);
}

static inline void ldh_a_indirect_c(struct gb *gb)
{
    gb->cpu.af.a = bus_read(gb, 0xff00 + gb->cpu.bc.c);
}

static inline void ldh_indirect_n_a(struct gb *gb, uint8_t n)
{
    bus_write(gb, 0xff00 + n, gb->cpu.af.a);
}

static inline void ldh_a_indirect_n(struct gb *gb, uint8_t n)
{
    gb->cpu.af.a = bus_read(gb, 0xff00 + n);
}

static inline void ld_indirect_nn_sp(struct gb *gb, uint16_t nn)
{
    bus_write(gb, nn, LSB(gb->cpu.sp));
    bus_write(gb, nn + 1, MSB(gb->cpu.sp));
}

static inline void push_rr(struct gb *gb, uint16_t rr)
{
    sm83_push_word(gb, rr);
}

static inline void ld_hl_sp_plus_i8(struct gb *gb, uint8_t i8)
{
    uint16_t carry_per_bit = (gb->cpu.sp + i8) ^ gb->cpu.sp ^ i8;

    gb->cpu.hl.val = gb->cpu.sp + (int8_t)i8;
    gb->cpu.af.flag.z = 0;
    gb->cpu.af.flag.n = 0;
    gb->cpu.af.flag.h = BIT(carry_per_bit, 4);
    gb->cpu.af.flag.c = BIT(carry_per_bit, 8);
}

void add(struct gb *gb, uint8_t b, uint8_t c)
{
    uint16_t res = gb->cpu.af.a + b + c;
    uint16_t carry_per_bit = res ^ gb->cpu.af.a ^ b;

    gb->cpu.af.a = res;
    toggle_znh(gb, res, 0, BIT(carry_per_bit, 4));
    gb->cpu.af.flag.c = BIT(carry_per_bit, 8);
}

void sub(struct gb *gb, uint8_t b, uint8_t c)
{
    uint16_t res = gb->cpu.af.a + ~b + 1 - c;
    uint16_t carry_per_bit = res ^ gb->cpu.af.a ^ ~b;

    gb->cpu.af.a = res;
    toggle_znh(gb, res, 1, !BIT(carry_per_bit, 4));
    gb->cpu.af.flag.c = !BIT(carry_per_bit, 8);
}

void cp(struct gb *gb, uint8_t b)
{
    uint16_t res = gb->cpu.af.a + ~b + 1;
    uint16_t carry_per_bit = gb->cpu.af.a ^ ~b ^ res;

    toggle_znh(gb, res, 1, !BIT(carry_per_bit, 4));
    gb->cpu.af.flag.c = !BIT(carry_per_bit, 8);
}

void inc_r(struct gb *gb, uint8_t *r)
{
    uint8_t res = *r + 1, carry_per_bit = res ^ *r ^ 1;

    *r += 1;
    toggle_znh(gb, res, 0, BIT(carry_per_bit, 4));
}

void inc_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);
    uint8_t res = val + 1, carry_per_bit = res ^ val ^ 1U;

    bus_write(gb, gb->cpu.hl.val, res);
    toggle_znh(gb, res, 0, BIT(carry_per_bit, 4));
}

void dec_r(struct gb *gb, uint8_t *r)
{
    uint8_t res = *r - 1, carry_per_bit = res ^ *r ^ 0xff;

    *r -= 1;
    toggle_znh(gb, res, 1, !BIT(carry_per_bit, 4));
}

void dec_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);
    uint8_t res = val - 1, carry_per_bit = res ^ val ^ 0xff;

    bus_write(gb, gb->cpu.hl.val, res);
    toggle_znh(gb, res, 1, !BIT(carry_per_bit, 4));
}

static inline void and(struct gb *gb, uint8_t b)
{
    gb->cpu.af.a &= b;
    toggle_znh(gb, gb->cpu.af.a, 0, 1);
    gb->cpu.af.flag.c = 0;
}

static inline void or(struct gb *gb, uint8_t b)
{
    gb->cpu.af.a |= b;
    toggle_znh(gb, gb->cpu.af.a, 0, 0);
    gb->cpu.af.flag.c = 0;
}

static inline void xor(struct gb *gb, uint8_t b)
{
    gb->cpu.af.a ^= b;
    toggle_znh(gb, gb->cpu.af.a, 0, 0);
    gb->cpu.af.flag.c = 0;
}

static inline void ccf(struct gb *gb)
{
    gb->cpu.af.flag.n = 0;
    gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = !gb->cpu.af.flag.c;
}

static inline void scf(struct gb *gb)
{
    gb->cpu.af.flag.n = 0;
    gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = 1;
}

void daa(struct gb *gb)
{
    uint8_t a = gb->cpu.af.a;
    if (!gb->cpu.af.flag.n) {
        if (gb->cpu.af.flag.h || (gb->cpu.af.a & 0x0f) > 0x09)
            a += 0x06;
        if (gb->cpu.af.flag.c || gb->cpu.af.a > 0x99) {
            a += 0x60;
            gb->cpu.af.flag.c = 1;
        }
    } else {
        if (gb->cpu.af.flag.h)
            a -= 0x06;
        if (gb->cpu.af.flag.c)
            a -= 0x60;
    }
    gb->cpu.af.flag.z = !a;
    gb->cpu.af.flag.h = 0;
    gb->cpu.af.a = a;
}

static inline void cpl(struct gb *gb)
{
    gb->cpu.af.a = ~gb->cpu.af.a;
    gb->cpu.af.flag.n = 1;
    gb->cpu.af.flag.h = 1;
}

void inc_rr(struct gb *gb, uint16_t *rr)
{
    *rr += 1; 
}

void dec_rr(struct gb *gb, uint16_t *rr)
{
    *rr -= 1; 
}

void add_hl_rr(struct gb *gb, uint16_t rr)
{
    uint32_t res = gb->cpu.hl.val + rr;
    uint32_t carry_per_bit = res ^ gb->cpu.hl.val ^ rr;

    gb->cpu.hl.val = res;
    gb->cpu.af.flag.n = 0;
    gb->cpu.af.flag.h = BIT(carry_per_bit, 12);
    gb->cpu.af.flag.c = BIT(carry_per_bit, 16);
}

void add_sp_i8(struct gb *gb, uint8_t i8)
{
    uint16_t carry_per_bit = (gb->cpu.sp + i8) ^ gb->cpu.sp ^ i8;

    gb->cpu.sp = gb->cpu.sp + (int8_t)i8;
    gb->cpu.af.flag.z = gb->cpu.af.flag.n = 0;
    gb->cpu.af.flag.h = BIT(carry_per_bit, 4);
    gb->cpu.af.flag.c = BIT(carry_per_bit, 8);
}

static inline void rlca(struct gb *gb)
{
    gb->cpu.af.flag.c = BIT(gb->cpu.af.a, 7);
    gb->cpu.af.a = (gb->cpu.af.a << 1) | gb->cpu.af.flag.c;
    gb->cpu.af.flag.z = gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0; 
}

static inline void rrca(struct gb *gb)
{
    gb->cpu.af.flag.c = BIT(gb->cpu.af.a, 0);
    gb->cpu.af.a = (gb->cpu.af.a >> 1) | (gb->cpu.af.flag.c << 7);
    gb->cpu.af.flag.z = gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0; 
}

void rla(struct gb *gb)
{
    uint8_t new_c = BIT(gb->cpu.af.a, 7);

    gb->cpu.af.a = (gb->cpu.af.a << 1) | gb->cpu.af.flag.c;
    gb->cpu.af.flag.c = new_c;
    gb->cpu.af.flag.z = gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0; 
}

void rra(struct gb *gb)
{
    uint8_t new_c = BIT(gb->cpu.af.a, 0);

    gb->cpu.af.a = (gb->cpu.af.a >> 1) | (gb->cpu.af.flag.c << 7);
    gb->cpu.af.flag.c = new_c;
    gb->cpu.af.flag.z = gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0; 
}

void rlc_r(struct gb *gb, uint8_t *r)
{
    gb->cpu.af.flag.c = BIT(*r, 7);
    *r = (*r << 1) | gb->cpu.af.flag.c;
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void rlc_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    gb->cpu.af.flag.c = BIT(val, 7);
    val = (val << 1) | gb->cpu.af.flag.c;
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void rrc_r(struct gb *gb, uint8_t *r)
{
    gb->cpu.af.flag.c = BIT(*r, 0);
    *r = (*r >> 1) | (gb->cpu.af.flag.c << 7);
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void rrc_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    gb->cpu.af.flag.c = BIT(val, 0);
    val = (val >> 1) | (gb->cpu.af.flag.c << 7);
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void rl_r(struct gb *gb, uint8_t *r)
{
    uint8_t new_c = BIT(*r, 7);

    *r = (*r << 1) | gb->cpu.af.flag.c;
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = new_c;
}

void rl_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val), new_c = BIT(val, 7);

    val = (val << 1) | gb->cpu.af.flag.c;
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = new_c;
}

void rr_r(struct gb *gb, uint8_t *r)
{
    uint8_t new_c = BIT(*r, 0);

    *r = (*r >> 1) | (gb->cpu.af.flag.c << 7);
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = new_c;
}

void rr_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val), new_c = BIT(val, 0);

    val = (val >> 1) | (gb->cpu.af.flag.c << 7);
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
    gb->cpu.af.flag.c = new_c;
}

void sla_r(struct gb *gb, uint8_t *r)
{
    gb->cpu.af.flag.c = BIT(*r, 7);
    *r <<= 1;
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void sla_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    gb->cpu.af.flag.c = BIT(val, 7);
    val <<= 1;
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void sra_r(struct gb *gb, uint8_t *r)
{
    gb->cpu.af.flag.c = BIT(*r, 0);
    *r = (*r & 0x80) | (*r >> 1);
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void sra_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    gb->cpu.af.flag.c = BIT(val, 0);
    val = (val & 0x80) | (val >> 1);
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void swap_r(struct gb *gb, uint8_t *r)
{
    *r = (*r >> 4) | (*r << 4);
    toggle_znh(gb, *r, 0, 0);
    gb->cpu.af.flag.c = 0;
}

void swap_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    val = (val >> 4) | (val << 4);
    bus_write(gb, gb->cpu.hl.val, val);
    toggle_znh(gb, val, 0, 0);
    gb->cpu.af.flag.c = 0;
}

void srl_r(struct gb *gb, uint8_t *r)
{
    gb->cpu.af.flag.c = BIT(*r, 0);
    *r >>= 1;
    gb->cpu.af.flag.z = !(*r);
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

void srl_indirect_hl(struct gb *gb)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    gb->cpu.af.flag.c = BIT(val, 0);
    val >>= 1;
    bus_write(gb, gb->cpu.hl.val, val);
    gb->cpu.af.flag.z = !val;
    gb->cpu.af.flag.n = gb->cpu.af.flag.h = 0;
}

static inline void bit_n_r(struct gb *gb, uint8_t n, uint8_t *r)
{
    toggle_znh(gb, BIT(*r, n), 0, 1);
}

void bit_n_indirect_hl(struct gb *gb, uint8_t n)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    toggle_znh(gb, BIT(val, n), 0, 1);
}

void res_n_r(struct gb *gb, uint8_t n, uint8_t *r)
{
    RES(*r, n); 
}

void res_n_indirect_hl(struct gb *gb, uint8_t n)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    RES(val, n);
    bus_write(gb, gb->cpu.hl.val, val);
}

void set_n_r(struct gb *gb, uint8_t n, uint8_t *r)
{
    SET(*r, n);
}

void set_n_indirect_hl(struct gb *gb, uint8_t n)
{
    uint8_t val = bus_read(gb, gb->cpu.hl.val);

    SET(val, n);
    bus_write(gb, gb->cpu.hl.val, val);
}

void jp(struct gb *gb, uint16_t nn, uint8_t offset, bool cond)
{
    if (cond) {
        gb->executed_cycle += 1;
        gb->cpu.pc = nn + (int8_t)offset;
    }
}

void call(struct gb *gb, uint16_t nn, bool cond)
{
    if (cond) {
        sm83_push_word(gb, gb->cpu.pc);
        gb->cpu.pc = nn;
        gb->executed_cycle += 3;
    }
}

void ret(struct gb *gb, uint8_t opcode, bool cond)
{
    if (cond) {
        uint16_t pc = sm83_pop_word(gb);
        gb->cpu.pc = pc;
        gb->executed_cycle += 3;
    }
}

void reti(struct gb *gb)
{
    ret(gb, 0xc9, 1);
    gb->cpu.ime = true;
}

void rst_n(struct gb *gb, uint8_t n)
{
    sm83_push_word(gb, gb->cpu.pc);
    gb->cpu.pc = n;
}

void halt(struct gb *gb)
{
    gb->mode = HALT;
    if (is_interrupt_pending(gb)) {
        // TODO: halt bug
        gb->mode = (!gb->cpu.ime) ? HALT_BUG : NORMAL;
    }
}

void stop(struct gb *gb)
{
    // TODO
}

void di(struct gb *gb)
{
    gb->cpu.ime = false;
}

void ei(struct gb *gb)
{
    // TODO: EI's effect is delayed by one instruction
    gb->cpu.ime = true;
}

int execute_cb_instructions(struct gb *gb, uint8_t opcode)
{
    switch (opcode) {
    case 0x00: rlc_r(gb, &gb->cpu.bc.b);                              break;
    case 0x01: rlc_r(gb, &gb->cpu.bc.c);                              break;
    case 0x02: rlc_r(gb, &gb->cpu.de.d);                              break;
    case 0x03: rlc_r(gb, &gb->cpu.de.e);                              break;
    case 0x04: rlc_r(gb, &gb->cpu.hl.h);                              break;
    case 0x05: rlc_r(gb, &gb->cpu.hl.l);                              break;
    case 0x06: rlc_indirect_hl(gb);                                     break;
    case 0x07: rlc_r(gb, &gb->cpu.af.a);                              break;
    case 0x08: rrc_r(gb, &gb->cpu.bc.b);                              break;
    case 0x09: rrc_r(gb, &gb->cpu.bc.c);                              break;
    case 0x0a: rrc_r(gb, &gb->cpu.de.d);                              break;
    case 0x0b: rrc_r(gb, &gb->cpu.de.e);                              break;
    case 0x0c: rrc_r(gb, &gb->cpu.hl.h);                              break;
    case 0x0d: rrc_r(gb, &gb->cpu.hl.l);                              break;
    case 0x0e: rrc_indirect_hl(gb);                                     break;
    case 0x0f: rrc_r(gb, &gb->cpu.af.a);                              break;
    case 0x10: rl_r(gb, &gb->cpu.bc.b);                               break;
    case 0x11: rl_r(gb, &gb->cpu.bc.c);                               break;
    case 0x12: rl_r(gb, &gb->cpu.de.d);                               break;
    case 0x13: rl_r(gb, &gb->cpu.de.e);                               break;
    case 0x14: rl_r(gb, &gb->cpu.hl.h);                               break;
    case 0x15: rl_r(gb, &gb->cpu.hl.l);                               break;
    case 0x16: rl_indirect_hl(gb);                                      break;
    case 0x17: rl_r(gb, &gb->cpu.af.a);                               break;
    case 0x18: rr_r(gb, &gb->cpu.bc.b);                               break;
    case 0x19: rr_r(gb, &gb->cpu.bc.c);                               break;
    case 0x1a: rr_r(gb, &gb->cpu.de.d);                               break;
    case 0x1b: rr_r(gb, &gb->cpu.de.e);                               break;
    case 0x1c: rr_r(gb, &gb->cpu.hl.h);                               break;
    case 0x1d: rr_r(gb, &gb->cpu.hl.l);                               break;
    case 0x1e: rr_indirect_hl(gb);                                      break;
    case 0x1f: rr_r(gb, &gb->cpu.af.a);                               break;
    case 0x20: sla_r(gb, &gb->cpu.bc.b);                              break;
    case 0x21: sla_r(gb, &gb->cpu.bc.c);                              break;
    case 0x22: sla_r(gb, &gb->cpu.de.d);                              break;
    case 0x23: sla_r(gb, &gb->cpu.de.e);                              break;
    case 0x24: sla_r(gb, &gb->cpu.hl.h);                              break;
    case 0x25: sla_r(gb, &gb->cpu.hl.l);                              break;
    case 0x26: sla_indirect_hl(gb);                                     break;
    case 0x27: sla_r(gb, &gb->cpu.af.a);                              break;
    case 0x28: sra_r(gb, &gb->cpu.bc.b);                              break;
    case 0x29: sra_r(gb, &gb->cpu.bc.c);                              break;
    case 0x2a: sra_r(gb, &gb->cpu.de.d);                              break;
    case 0x2b: sra_r(gb, &gb->cpu.de.e);                              break;
    case 0x2c: sra_r(gb, &gb->cpu.hl.h);                              break;
    case 0x2d: sra_r(gb, &gb->cpu.hl.l);                              break;
    case 0x2e: sra_indirect_hl(gb);                                     break;
    case 0x2f: sra_r(gb, &gb->cpu.af.a);                              break;
    case 0x30: swap_r(gb, &gb->cpu.bc.b);                             break;
    case 0x31: swap_r(gb, &gb->cpu.bc.c);                             break;
    case 0x32: swap_r(gb, &gb->cpu.de.d);                             break;
    case 0x33: swap_r(gb, &gb->cpu.de.e);                             break;
    case 0x34: swap_r(gb, &gb->cpu.hl.h);                             break;
    case 0x35: swap_r(gb, &gb->cpu.hl.l);                             break;
    case 0x36: swap_indirect_hl(gb);                                    break;
    case 0x37: swap_r(gb, &gb->cpu.af.a);                             break;
    case 0x38: srl_r(gb, &gb->cpu.bc.b);                              break;
    case 0x39: srl_r(gb, &gb->cpu.bc.c);                              break;
    case 0x3a: srl_r(gb, &gb->cpu.de.d);                              break;
    case 0x3b: srl_r(gb, &gb->cpu.de.e);                              break;
    case 0x3c: srl_r(gb, &gb->cpu.hl.h);                              break;
    case 0x3d: srl_r(gb, &gb->cpu.hl.l);                              break;
    case 0x3e: srl_indirect_hl(gb);                                     break;
    case 0x3f: srl_r(gb, &gb->cpu.af.a);                              break;
    case 0x40: bit_n_r(gb, 0, &gb->cpu.bc.b);                         break;
    case 0x41: bit_n_r(gb, 0, &gb->cpu.bc.c);                         break;
    case 0x42: bit_n_r(gb, 0, &gb->cpu.de.d);                         break;
    case 0x43: bit_n_r(gb, 0, &gb->cpu.de.e);                         break;
    case 0x44: bit_n_r(gb, 0, &gb->cpu.hl.h);                         break;
    case 0x45: bit_n_r(gb, 0, &gb->cpu.hl.l);                         break;
    case 0x46: bit_n_indirect_hl(gb, 0);                                break;
    case 0x47: bit_n_r(gb, 0, &gb->cpu.af.a);                         break;
    case 0x48: bit_n_r(gb, 1, &gb->cpu.bc.b);                         break;
    case 0x49: bit_n_r(gb, 1, &gb->cpu.bc.c);                         break;
    case 0x4a: bit_n_r(gb, 1, &gb->cpu.de.d);                         break;
    case 0x4b: bit_n_r(gb, 1, &gb->cpu.de.e);                         break;
    case 0x4c: bit_n_r(gb, 1, &gb->cpu.hl.h);                         break;
    case 0x4d: bit_n_r(gb, 1, &gb->cpu.hl.l);                         break;
    case 0x4e: bit_n_indirect_hl(gb, 1);                                break;
    case 0x4f: bit_n_r(gb, 1, &gb->cpu.af.a);                         break;
    case 0x50: bit_n_r(gb, 2, &gb->cpu.bc.b);                         break;
    case 0x51: bit_n_r(gb, 2, &gb->cpu.bc.c);                         break;
    case 0x52: bit_n_r(gb, 2, &gb->cpu.de.d);                         break;
    case 0x53: bit_n_r(gb, 2, &gb->cpu.de.e);                         break;
    case 0x54: bit_n_r(gb, 2, &gb->cpu.hl.h);                         break;
    case 0x55: bit_n_r(gb, 2, &gb->cpu.hl.l);                         break;
    case 0x56: bit_n_indirect_hl(gb, 2);                                break;
    case 0x57: bit_n_r(gb, 2, &gb->cpu.af.a);                         break;
    case 0x58: bit_n_r(gb, 3, &gb->cpu.bc.b);                         break;
    case 0x59: bit_n_r(gb, 3, &gb->cpu.bc.c);                         break;
    case 0x5a: bit_n_r(gb, 3, &gb->cpu.de.d);                         break;
    case 0x5b: bit_n_r(gb, 3, &gb->cpu.de.e);                         break;
    case 0x5c: bit_n_r(gb, 3, &gb->cpu.hl.h);                         break;
    case 0x5d: bit_n_r(gb, 3, &gb->cpu.hl.l);                         break;
    case 0x5e: bit_n_indirect_hl(gb, 3);                                break;
    case 0x5f: bit_n_r(gb, 3, &gb->cpu.af.a);                         break;
    case 0x60: bit_n_r(gb, 4, &gb->cpu.bc.b);                         break;
    case 0x61: bit_n_r(gb, 4, &gb->cpu.bc.c);                         break;
    case 0x62: bit_n_r(gb, 4, &gb->cpu.de.d);                         break;
    case 0x63: bit_n_r(gb, 4, &gb->cpu.de.e);                         break;
    case 0x64: bit_n_r(gb, 4, &gb->cpu.hl.h);                         break;
    case 0x65: bit_n_r(gb, 4, &gb->cpu.hl.l);                         break;
    case 0x66: bit_n_indirect_hl(gb, 4);                                break;
    case 0x67: bit_n_r(gb, 4, &gb->cpu.af.a);                         break;
    case 0x68: bit_n_r(gb, 5, &gb->cpu.bc.b);                         break;
    case 0x69: bit_n_r(gb, 5, &gb->cpu.bc.c);                         break;
    case 0x6a: bit_n_r(gb, 5, &gb->cpu.de.d);                         break;
    case 0x6b: bit_n_r(gb, 5, &gb->cpu.de.e);                         break;
    case 0x6c: bit_n_r(gb, 5, &gb->cpu.hl.h);                         break;
    case 0x6d: bit_n_r(gb, 5, &gb->cpu.hl.l);                         break;
    case 0x6e: bit_n_indirect_hl(gb, 5);                                break;
    case 0x6f: bit_n_r(gb, 5, &gb->cpu.af.a);                         break;
    case 0x70: bit_n_r(gb, 6, &gb->cpu.bc.b);                         break;
    case 0x71: bit_n_r(gb, 6, &gb->cpu.bc.c);                         break;
    case 0x72: bit_n_r(gb, 6, &gb->cpu.de.d);                         break;
    case 0x73: bit_n_r(gb, 6, &gb->cpu.de.e);                         break;
    case 0x74: bit_n_r(gb, 6, &gb->cpu.hl.h);                         break;
    case 0x75: bit_n_r(gb, 6, &gb->cpu.hl.l);                         break;
    case 0x76: bit_n_indirect_hl(gb, 6);                                break;
    case 0x77: bit_n_r(gb, 6, &gb->cpu.af.a);                         break;
    case 0x78: bit_n_r(gb, 7, &gb->cpu.bc.b);                         break;
    case 0x79: bit_n_r(gb, 7, &gb->cpu.bc.c);                         break;
    case 0x7a: bit_n_r(gb, 7, &gb->cpu.de.d);                         break;
    case 0x7b: bit_n_r(gb, 7, &gb->cpu.de.e);                         break;
    case 0x7c: bit_n_r(gb, 7, &gb->cpu.hl.h);                         break;
    case 0x7d: bit_n_r(gb, 7, &gb->cpu.hl.l);                         break;
    case 0x7e: bit_n_indirect_hl(gb, 7);                                break;
    case 0x7f: bit_n_r(gb, 7, &gb->cpu.af.a);                         break;
    case 0x80: res_n_r(gb, 0, &gb->cpu.bc.b);                         break;
    case 0x81: res_n_r(gb, 0, &gb->cpu.bc.c);                         break;
    case 0x82: res_n_r(gb, 0, &gb->cpu.de.d);                         break;
    case 0x83: res_n_r(gb, 0, &gb->cpu.de.e);                         break;
    case 0x84: res_n_r(gb, 0, &gb->cpu.hl.h);                         break;
    case 0x85: res_n_r(gb, 0, &gb->cpu.hl.l);                         break;
    case 0x86: res_n_indirect_hl(gb, 0);                                break;
    case 0x87: res_n_r(gb, 0, &gb->cpu.af.a);                         break;
    case 0x88: res_n_r(gb, 1, &gb->cpu.bc.b);                         break;
    case 0x89: res_n_r(gb, 1, &gb->cpu.bc.c);                         break;
    case 0x8a: res_n_r(gb, 1, &gb->cpu.de.d);                         break;
    case 0x8b: res_n_r(gb, 1, &gb->cpu.de.e);                         break;
    case 0x8c: res_n_r(gb, 1, &gb->cpu.hl.h);                         break;
    case 0x8d: res_n_r(gb, 1, &gb->cpu.hl.l);                         break;
    case 0x8e: res_n_indirect_hl(gb, 1);                                break;
    case 0x8f: res_n_r(gb, 1, &gb->cpu.af.a);                         break;
    case 0x90: res_n_r(gb, 2, &gb->cpu.bc.b);                         break;
    case 0x91: res_n_r(gb, 2, &gb->cpu.bc.c);                         break;
    case 0x92: res_n_r(gb, 2, &gb->cpu.de.d);                         break;
    case 0x93: res_n_r(gb, 2, &gb->cpu.de.e);                         break;
    case 0x94: res_n_r(gb, 2, &gb->cpu.hl.h);                         break;
    case 0x95: res_n_r(gb, 2, &gb->cpu.hl.l);                         break;
    case 0x96: res_n_indirect_hl(gb, 2);                                break;
    case 0x97: res_n_r(gb, 2, &gb->cpu.af.a);                         break;
    case 0x98: res_n_r(gb, 3, &gb->cpu.bc.b);                         break;
    case 0x99: res_n_r(gb, 3, &gb->cpu.bc.c);                         break;
    case 0x9a: res_n_r(gb, 3, &gb->cpu.de.d);                         break;
    case 0x9b: res_n_r(gb, 3, &gb->cpu.de.e);                         break;
    case 0x9c: res_n_r(gb, 3, &gb->cpu.hl.h);                         break;
    case 0x9d: res_n_r(gb, 3, &gb->cpu.hl.l);                         break;
    case 0x9e: res_n_indirect_hl(gb, 3);                                break;
    case 0x9f: res_n_r(gb, 3, &gb->cpu.af.a);                         break;
    case 0xa0: res_n_r(gb, 4, &gb->cpu.bc.b);                         break;
    case 0xa1: res_n_r(gb, 4, &gb->cpu.bc.c);                         break;
    case 0xa2: res_n_r(gb, 4, &gb->cpu.de.d);                         break;
    case 0xa3: res_n_r(gb, 4, &gb->cpu.de.e);                         break;
    case 0xa4: res_n_r(gb, 4, &gb->cpu.hl.h);                         break;
    case 0xa5: res_n_r(gb, 4, &gb->cpu.hl.l);                         break;
    case 0xa6: res_n_indirect_hl(gb, 4);                                break;
    case 0xa7: res_n_r(gb, 4, &gb->cpu.af.a);                         break;
    case 0xa8: res_n_r(gb, 5, &gb->cpu.bc.b);                         break;
    case 0xa9: res_n_r(gb, 5, &gb->cpu.bc.c);                         break;
    case 0xaa: res_n_r(gb, 5, &gb->cpu.de.d);                         break;
    case 0xab: res_n_r(gb, 5, &gb->cpu.de.e);                         break;
    case 0xac: res_n_r(gb, 5, &gb->cpu.hl.h);                         break;
    case 0xad: res_n_r(gb, 5, &gb->cpu.hl.l);                         break;
    case 0xae: res_n_indirect_hl(gb, 5);                                break;
    case 0xaf: res_n_r(gb, 5, &gb->cpu.af.a);                         break;
    case 0xb0: res_n_r(gb, 6, &gb->cpu.bc.b);                         break;
    case 0xb1: res_n_r(gb, 6, &gb->cpu.bc.c);                         break;
    case 0xb2: res_n_r(gb, 6, &gb->cpu.de.d);                         break;
    case 0xb3: res_n_r(gb, 6, &gb->cpu.de.e);                         break;
    case 0xb4: res_n_r(gb, 6, &gb->cpu.hl.h);                         break;
    case 0xb5: res_n_r(gb, 6, &gb->cpu.hl.l);                         break;
    case 0xb6: res_n_indirect_hl(gb, 6);                                break;
    case 0xb7: res_n_r(gb, 6, &gb->cpu.af.a);                         break;
    case 0xb8: res_n_r(gb, 7, &gb->cpu.bc.b);                         break;
    case 0xb9: res_n_r(gb, 7, &gb->cpu.bc.c);                         break;
    case 0xba: res_n_r(gb, 7, &gb->cpu.de.d);                         break;
    case 0xbb: res_n_r(gb, 7, &gb->cpu.de.e);                         break;
    case 0xbc: res_n_r(gb, 7, &gb->cpu.hl.h);                         break;
    case 0xbd: res_n_r(gb, 7, &gb->cpu.hl.l);                         break;
    case 0xbe: res_n_indirect_hl(gb, 7);                                break;
    case 0xbf: res_n_r(gb, 7, &gb->cpu.af.a);                         break;
    case 0xc0: set_n_r(gb, 0, &gb->cpu.bc.b);                         break;
    case 0xc1: set_n_r(gb, 0, &gb->cpu.bc.c);                         break;
    case 0xc2: set_n_r(gb, 0, &gb->cpu.de.d);                         break;
    case 0xc3: set_n_r(gb, 0, &gb->cpu.de.e);                         break;
    case 0xc4: set_n_r(gb, 0, &gb->cpu.hl.h);                         break;
    case 0xc5: set_n_r(gb, 0, &gb->cpu.hl.l);                         break;
    case 0xc6: set_n_indirect_hl(gb, 0);                                break;
    case 0xc7: set_n_r(gb, 0, &gb->cpu.af.a);                         break;
    case 0xc8: set_n_r(gb, 1, &gb->cpu.bc.b);                         break;
    case 0xc9: set_n_r(gb, 1, &gb->cpu.bc.c);                         break;
    case 0xca: set_n_r(gb, 1, &gb->cpu.de.d);                         break;
    case 0xcb: set_n_r(gb, 1, &gb->cpu.de.e);                         break;
    case 0xcc: set_n_r(gb, 1, &gb->cpu.hl.h);                         break;
    case 0xcd: set_n_r(gb, 1, &gb->cpu.hl.l);                         break;
    case 0xce: set_n_indirect_hl(gb, 1);                                break;
    case 0xcf: set_n_r(gb, 1, &gb->cpu.af.a);                         break;
    case 0xd0: set_n_r(gb, 2, &gb->cpu.bc.b);                         break;
    case 0xd1: set_n_r(gb, 2, &gb->cpu.bc.c);                         break;
    case 0xd2: set_n_r(gb, 2, &gb->cpu.de.d);                         break;
    case 0xd3: set_n_r(gb, 2, &gb->cpu.de.e);                         break;
    case 0xd4: set_n_r(gb, 2, &gb->cpu.hl.h);                         break;
    case 0xd5: set_n_r(gb, 2, &gb->cpu.hl.l);                         break;
    case 0xd6: set_n_indirect_hl(gb, 2);                                break;
    case 0xd7: set_n_r(gb, 2, &gb->cpu.af.a);                         break;
    case 0xd8: set_n_r(gb, 3, &gb->cpu.bc.b);                         break;
    case 0xd9: set_n_r(gb, 3, &gb->cpu.bc.c);                         break;
    case 0xda: set_n_r(gb, 3, &gb->cpu.de.d);                         break;
    case 0xdb: set_n_r(gb, 3, &gb->cpu.de.e);                         break;
    case 0xdc: set_n_r(gb, 3, &gb->cpu.hl.h);                         break;
    case 0xdd: set_n_r(gb, 3, &gb->cpu.hl.l);                         break;
    case 0xde: set_n_indirect_hl(gb, 3);                                break;
    case 0xdf: set_n_r(gb, 3, &gb->cpu.af.a);                         break;
    case 0xe0: set_n_r(gb, 4, &gb->cpu.bc.b);                         break;
    case 0xe1: set_n_r(gb, 4, &gb->cpu.bc.c);                         break;
    case 0xe2: set_n_r(gb, 4, &gb->cpu.de.d);                         break;
    case 0xe3: set_n_r(gb, 4, &gb->cpu.de.e);                         break;
    case 0xe4: set_n_r(gb, 4, &gb->cpu.hl.h);                         break;
    case 0xe5: set_n_r(gb, 4, &gb->cpu.hl.l);                         break;
    case 0xe6: set_n_indirect_hl(gb, 4);                                break;
    case 0xe7: set_n_r(gb, 4, &gb->cpu.af.a);                         break;
    case 0xe8: set_n_r(gb, 5, &gb->cpu.bc.b);                         break;
    case 0xe9: set_n_r(gb, 5, &gb->cpu.bc.c);                         break;
    case 0xea: set_n_r(gb, 5, &gb->cpu.de.d);                         break;
    case 0xeb: set_n_r(gb, 5, &gb->cpu.de.e);                         break;
    case 0xec: set_n_r(gb, 5, &gb->cpu.hl.h);                         break;
    case 0xed: set_n_r(gb, 5, &gb->cpu.hl.l);                         break;
    case 0xee: set_n_indirect_hl(gb, 5);                                break;
    case 0xef: set_n_r(gb, 5, &gb->cpu.af.a);                         break;
    case 0xf0: set_n_r(gb, 6, &gb->cpu.bc.b);                         break;
    case 0xf1: set_n_r(gb, 6, &gb->cpu.bc.c);                         break;
    case 0xf2: set_n_r(gb, 6, &gb->cpu.de.d);                         break;
    case 0xf3: set_n_r(gb, 6, &gb->cpu.de.e);                         break;
    case 0xf4: set_n_r(gb, 6, &gb->cpu.hl.h);                         break;
    case 0xf5: set_n_r(gb, 6, &gb->cpu.hl.l);                         break;
    case 0xf6: set_n_indirect_hl(gb, 6);                                break;
    case 0xf7: set_n_r(gb, 6, &gb->cpu.af.a);                         break;
    case 0xf8: set_n_r(gb, 7, &gb->cpu.bc.b);                         break;
    case 0xf9: set_n_r(gb, 7, &gb->cpu.bc.c);                         break;
    case 0xfa: set_n_r(gb, 7, &gb->cpu.de.d);                         break;
    case 0xfb: set_n_r(gb, 7, &gb->cpu.de.e);                         break;
    case 0xfc: set_n_r(gb, 7, &gb->cpu.hl.h);                         break;
    case 0xfd: set_n_r(gb, 7, &gb->cpu.hl.l);                         break;
    case 0xfe: set_n_indirect_hl(gb, 7);                                break;
    case 0xff: set_n_r(gb, 7, &gb->cpu.af.a);                         break;
    default:
        break;
    }
    return cb_instr_cycle[opcode];
}

int sm83_step(struct gb *gb)
{
    uint8_t opcode = sm83_fetch_byte(gb);
    gb->executed_cycle = instr_cycle[opcode];

    switch (opcode) {
    case 0x00:                                                          break;
    case 0x01: gb->cpu.bc.val = sm83_fetch_word(gb);                   break;
    case 0x02: bus_write(gb, gb->cpu.bc.val, gb->cpu.af.a);          break;
    case 0x03: inc_rr(gb, &gb->cpu.bc.val);                            break;
    case 0x04: inc_r(gb, &gb->cpu.bc.b);                              break;
    case 0x05: dec_r(gb, &gb->cpu.bc.b);                              break;
    case 0x06: gb->cpu.bc.b = sm83_fetch_byte(gb);                    break;
    case 0x07: rlca(gb);                                                break;
    case 0x08: ld_indirect_nn_sp(gb, sm83_fetch_word(gb));              break;
    case 0x09: add_hl_rr(gb, gb->cpu.bc.val);                          break;
    case 0x0a: gb->cpu.af.a = bus_read(gb, gb->cpu.bc.val);          break;
    case 0x0b: dec_rr(gb, &gb->cpu.bc.val);                            break;
    case 0x0c: inc_r(gb, &gb->cpu.bc.c);                              break;
    case 0x0d: dec_r(gb, &gb->cpu.bc.c);                              break;
    case 0x0e: gb->cpu.bc.c = sm83_fetch_byte(gb);                    break;
    case 0x0f: rrca(gb);                                                break;
    case 0x10: stop(gb);                                                break;
    case 0x11: gb->cpu.de.val = sm83_fetch_word(gb);                   break;
    case 0x12: bus_write(gb, gb->cpu.de.val, gb->cpu.af.a);          break;
    case 0x13: inc_rr(gb, &gb->cpu.de.val);                            break;
    case 0x14: inc_r(gb, &gb->cpu.de.d);                              break;
    case 0x15: dec_r(gb, &gb->cpu.de.d);                              break;
    case 0x16: gb->cpu.de.d = sm83_fetch_byte(gb);                    break;
    case 0x17: rla(gb);                                                 break;
    case 0x18: jp(gb, gb->cpu.pc, sm83_fetch_byte(gb), 1);         break;
    case 0x19: add_hl_rr(gb, gb->cpu.de.val);                          break;
    case 0x1a: gb->cpu.af.a = bus_read(gb, gb->cpu.de.val);          break;
    case 0x1b: dec_rr(gb, &gb->cpu.de.val);                            break;
    case 0x1c: inc_r(gb, &gb->cpu.de.e);                              break;
    case 0x1d: dec_r(gb, &gb->cpu.de.e);                              break;
    case 0x1e: gb->cpu.de.e = sm83_fetch_byte(gb);                    break;
    case 0x1f: rra(gb);                                                 break;
    case 0x20: jp(gb, gb->cpu.pc, sm83_fetch_byte(gb), !gb->cpu.af.flag.z);  break;
    case 0x21: gb->cpu.hl.val = sm83_fetch_word(gb);                   break;
    case 0x22: bus_write(gb, gb->cpu.hl.val++, gb->cpu.af.a);        break;
    case 0x23: inc_rr(gb, &gb->cpu.hl.val);                            break;
    case 0x24: inc_r(gb, &gb->cpu.hl.h);                              break;
    case 0x25: dec_r(gb, &gb->cpu.hl.h);                              break;
    case 0x26: gb->cpu.hl.h = sm83_fetch_byte(gb);                    break;
    case 0x27: daa(gb);                                                 break;
    case 0x28: jp(gb, gb->cpu.pc, sm83_fetch_byte(gb), gb->cpu.af.flag.z);  break;
    case 0x29: add_hl_rr(gb, gb->cpu.hl.val);                          break;
    case 0x2a: gb->cpu.af.a = bus_read(gb, gb->cpu.hl.val++);        break;
    case 0x2b: dec_rr(gb, &gb->cpu.hl.val);                            break;
    case 0x2c: inc_r(gb, &gb->cpu.hl.l);                              break;
    case 0x2d: dec_r(gb, &gb->cpu.hl.l);                              break;
    case 0x2e: gb->cpu.hl.l = sm83_fetch_byte(gb);                    break;
    case 0x2f: cpl(gb);                                                 break;
    case 0x30: jp(gb, gb->cpu.pc, sm83_fetch_byte(gb), !gb->cpu.af.flag.c);  break;
    case 0x31: gb->cpu.sp = sm83_fetch_word(gb);                   break;
    case 0x32: bus_write(gb, gb->cpu.hl.val--, gb->cpu.af.a);        break;
    case 0x33: inc_rr(gb, &gb->cpu.sp);                            break;
    case 0x34: inc_indirect_hl(gb);                                     break;
    case 0x35: dec_indirect_hl(gb);                                     break;
    case 0x36: ld_indirect_hl_n(gb, sm83_fetch_byte(gb));               break;
    case 0x37: scf(gb);                                                 break;
    case 0x38: jp(gb, gb->cpu.pc, sm83_fetch_byte(gb), gb->cpu.af.flag.c);  break;
    case 0x39: add_hl_rr(gb, gb->cpu.sp);                          break;
    case 0x3a: gb->cpu.af.a = bus_read(gb, gb->cpu.hl.val--);        break;
    case 0x3b: dec_rr(gb, &gb->cpu.sp);                            break;
    case 0x3c: inc_r(gb, &gb->cpu.af.a);                              break;
    case 0x3d: dec_r(gb, &gb->cpu.af.a);                              break;
    case 0x3e: gb->cpu.af.a = sm83_fetch_byte(gb);                    break;
    case 0x3f: ccf(gb);                                                 break;
    case 0x40: gb->cpu.bc.b = gb->cpu.bc.b;                         break;
    case 0x41: gb->cpu.bc.b = gb->cpu.bc.c;                         break;
    case 0x42: gb->cpu.bc.b = gb->cpu.de.d;                         break;
    case 0x43: gb->cpu.bc.b = gb->cpu.de.e;                         break;
    case 0x44: gb->cpu.bc.b = gb->cpu.hl.h;                         break;
    case 0x45: gb->cpu.bc.b = gb->cpu.hl.l;                         break;
    case 0x46: gb->cpu.bc.b = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x47: gb->cpu.bc.b = gb->cpu.af.a;                         break;
    case 0x48: gb->cpu.bc.c = gb->cpu.bc.b;                         break;
    case 0x49: gb->cpu.bc.c = gb->cpu.bc.c;                         break;
    case 0x4a: gb->cpu.bc.c = gb->cpu.de.d;                         break;
    case 0x4b: gb->cpu.bc.c = gb->cpu.de.e;                         break;
    case 0x4c: gb->cpu.bc.c = gb->cpu.hl.h;                         break;
    case 0x4d: gb->cpu.bc.c = gb->cpu.hl.l;                         break;
    case 0x4e: gb->cpu.bc.c = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x4f: gb->cpu.bc.c = gb->cpu.af.a;                         break;
    case 0x50: gb->cpu.de.d = gb->cpu.bc.b;                         break;
    case 0x51: gb->cpu.de.d = gb->cpu.bc.c;                         break;
    case 0x52: gb->cpu.de.d = gb->cpu.de.d;                         break;
    case 0x53: gb->cpu.de.d = gb->cpu.de.e;                         break;
    case 0x54: gb->cpu.de.d = gb->cpu.hl.h;                         break;
    case 0x55: gb->cpu.de.d = gb->cpu.hl.l;                         break;
    case 0x56: gb->cpu.de.d = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x57: gb->cpu.de.d = gb->cpu.af.a;                         break;
    case 0x58: gb->cpu.de.e = gb->cpu.bc.b;                         break;
    case 0x59: gb->cpu.de.e = gb->cpu.bc.c;                         break;
    case 0x5a: gb->cpu.de.e = gb->cpu.de.d;                         break;
    case 0x5b: gb->cpu.de.e = gb->cpu.de.e;                         break;
    case 0x5c: gb->cpu.de.e = gb->cpu.hl.h;                         break;
    case 0x5d: gb->cpu.de.e = gb->cpu.hl.l;                         break;
    case 0x5e: gb->cpu.de.e = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x5f: gb->cpu.de.e = gb->cpu.af.a;                         break;
    case 0x60: gb->cpu.hl.h = gb->cpu.bc.b;                         break;
    case 0x61: gb->cpu.hl.h = gb->cpu.bc.c;                         break;
    case 0x62: gb->cpu.hl.h = gb->cpu.de.d;                         break;
    case 0x63: gb->cpu.hl.h = gb->cpu.de.e;                         break;
    case 0x64: gb->cpu.hl.h = gb->cpu.hl.h;                         break;
    case 0x65: gb->cpu.hl.h = gb->cpu.hl.l;                         break;
    case 0x66: gb->cpu.hl.h = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x67: gb->cpu.hl.h = gb->cpu.af.a;                         break;
    case 0x68: gb->cpu.hl.l = gb->cpu.bc.b;                         break;
    case 0x69: gb->cpu.hl.l = gb->cpu.bc.c;                         break;
    case 0x6a: gb->cpu.hl.l = gb->cpu.de.d;                         break;
    case 0x6b: gb->cpu.hl.l = gb->cpu.de.e;                         break;
    case 0x6c: gb->cpu.hl.l = gb->cpu.hl.h;                         break;
    case 0x6d: gb->cpu.hl.l = gb->cpu.hl.l;                         break;
    case 0x6e: gb->cpu.hl.l = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x6f: gb->cpu.hl.l = gb->cpu.af.a;                         break;
    case 0x70: bus_write(gb, gb->cpu.hl.val, gb->cpu.bc.b);          break;
    case 0x71: bus_write(gb, gb->cpu.hl.val, gb->cpu.bc.c);          break;
    case 0x72: bus_write(gb, gb->cpu.hl.val, gb->cpu.de.d);          break;
    case 0x73: bus_write(gb, gb->cpu.hl.val, gb->cpu.de.e);          break;
    case 0x74: bus_write(gb, gb->cpu.hl.val, gb->cpu.hl.h);          break;
    case 0x75: bus_write(gb, gb->cpu.hl.val, gb->cpu.hl.l);          break;
    case 0x76: halt(gb);                                                break;
    case 0x77: bus_write(gb, gb->cpu.hl.val, gb->cpu.af.a);          break;
    case 0x78: gb->cpu.af.a = gb->cpu.bc.b;                         break;
    case 0x79: gb->cpu.af.a = gb->cpu.bc.c;                         break;
    case 0x7a: gb->cpu.af.a = gb->cpu.de.d;                         break;
    case 0x7b: gb->cpu.af.a = gb->cpu.de.e;                         break;
    case 0x7c: gb->cpu.af.a = gb->cpu.hl.h;                         break;
    case 0x7d: gb->cpu.af.a = gb->cpu.hl.l;                         break;
    case 0x7e: gb->cpu.af.a = bus_read(gb, gb->cpu.hl.val);          break;
    case 0x7f: gb->cpu.af.a = gb->cpu.af.a;                         break;
    case 0x80: add(gb, gb->cpu.bc.b, 0);                              break;
    case 0x81: add(gb, gb->cpu.bc.c, 0);                              break;
    case 0x82: add(gb, gb->cpu.de.d, 0);                              break;
    case 0x83: add(gb, gb->cpu.de.e, 0);                              break;
    case 0x84: add(gb, gb->cpu.hl.h, 0);                              break;
    case 0x85: add(gb, gb->cpu.hl.l, 0);                              break;
    case 0x86: add(gb, bus_read(gb, gb->cpu.hl.val), 0);               break;
    case 0x87: add(gb, gb->cpu.af.a, 0);                              break;
    case 0x88: add(gb, gb->cpu.bc.b, gb->cpu.af.flag.c);            break;
    case 0x89: add(gb, gb->cpu.bc.c, gb->cpu.af.flag.c);            break;
    case 0x8a: add(gb, gb->cpu.de.d, gb->cpu.af.flag.c);            break;
    case 0x8b: add(gb, gb->cpu.de.e, gb->cpu.af.flag.c);            break;
    case 0x8c: add(gb, gb->cpu.hl.h, gb->cpu.af.flag.c);            break;
    case 0x8d: add(gb, gb->cpu.hl.l, gb->cpu.af.flag.c);            break;
    case 0x8e: add(gb, bus_read(gb, gb->cpu.hl.val), gb->cpu.af.flag.c); break;
    case 0x8f: add(gb, gb->cpu.af.a, gb->cpu.af.flag.c);            break;
    case 0x90: sub(gb, gb->cpu.bc.b, 0);                              break;
    case 0x91: sub(gb, gb->cpu.bc.c, 0);                              break;
    case 0x92: sub(gb, gb->cpu.de.d, 0);                              break;
    case 0x93: sub(gb, gb->cpu.de.e, 0);                              break;
    case 0x94: sub(gb, gb->cpu.hl.h, 0);                              break;
    case 0x95: sub(gb, gb->cpu.hl.l, 0);                              break;
    case 0x96: sub(gb, bus_read(gb, gb->cpu.hl.val), 0);               break;
    case 0x97: sub(gb, gb->cpu.af.a, 0);                              break;
    case 0x98: sub(gb, gb->cpu.bc.b, gb->cpu.af.flag.c);            break;
    case 0x99: sub(gb, gb->cpu.bc.c, gb->cpu.af.flag.c);            break;
    case 0x9a: sub(gb, gb->cpu.de.d, gb->cpu.af.flag.c);            break;
    case 0x9b: sub(gb, gb->cpu.de.e, gb->cpu.af.flag.c);            break;
    case 0x9c: sub(gb, gb->cpu.hl.h, gb->cpu.af.flag.c);            break;
    case 0x9d: sub(gb, gb->cpu.hl.l, gb->cpu.af.flag.c);            break;
    case 0x9e: sub(gb, bus_read(gb, gb->cpu.hl.val), gb->cpu.af.flag.c); break;
    case 0x9f: sub(gb, gb->cpu.af.a, gb->cpu.af.flag.c);            break;
    case 0xa0: and(gb, gb->cpu.bc.b);                                 break;
    case 0xa1: and(gb, gb->cpu.bc.c);                                 break;
    case 0xa2: and(gb, gb->cpu.de.d);                                 break;
    case 0xa3: and(gb, gb->cpu.de.e);                                 break;
    case 0xa4: and(gb, gb->cpu.hl.h);                                 break;
    case 0xa5: and(gb, gb->cpu.hl.l);                                 break;
    case 0xa6: and(gb, bus_read(gb, gb->cpu.hl.val));                  break;
    case 0xa7: and(gb, gb->cpu.af.a);                                 break;
    case 0xa8: xor(gb, gb->cpu.bc.b);                                 break;
    case 0xa9: xor(gb, gb->cpu.bc.c);                                 break;
    case 0xaa: xor(gb, gb->cpu.de.d);                                 break;
    case 0xab: xor(gb, gb->cpu.de.e);                                 break;
    case 0xac: xor(gb, gb->cpu.hl.h);                                 break;
    case 0xad: xor(gb, gb->cpu.hl.l);                                 break;
    case 0xae: xor(gb, bus_read(gb, gb->cpu.hl.val));                  break;
    case 0xaf: xor(gb, gb->cpu.af.a);                                 break;
    case 0xb0: or(gb, gb->cpu.bc.b);                                  break;
    case 0xb1: or(gb, gb->cpu.bc.c);                                  break;
    case 0xb2: or(gb, gb->cpu.de.d);                                  break;
    case 0xb3: or(gb, gb->cpu.de.e);                                  break;
    case 0xb4: or(gb, gb->cpu.hl.h);                                  break;
    case 0xb5: or(gb, gb->cpu.hl.l);                                  break;
    case 0xb6: or(gb, bus_read(gb, gb->cpu.hl.val));                   break;
    case 0xb7: or(gb, gb->cpu.af.a);                                  break;
    case 0xb8: cp(gb, gb->cpu.bc.b);                                  break;
    case 0xb9: cp(gb, gb->cpu.bc.c);                                  break;
    case 0xba: cp(gb, gb->cpu.de.d);                                  break;
    case 0xbb: cp(gb, gb->cpu.de.e);                                  break;
    case 0xbc: cp(gb, gb->cpu.hl.h);                                  break;
    case 0xbd: cp(gb, gb->cpu.hl.l);                                  break;
    case 0xbe: cp(gb, bus_read(gb, gb->cpu.hl.val));                   break;
    case 0xbf: cp(gb, gb->cpu.af.a);                                  break;
    case 0xc0: ret(gb, opcode, !gb->cpu.af.flag.z);                   break;
    case 0xc1: gb->cpu.bc.val = sm83_pop_word(gb);                     break;
    case 0xc2: jp(gb, sm83_fetch_word(gb), 0, !gb->cpu.af.flag.z);    break;
    case 0xc3: jp(gb, sm83_fetch_word(gb), 0, 1);                       break; 
    case 0xc4: call(gb, sm83_fetch_word(gb), !gb->cpu.af.flag.z);     break;
    case 0xc5: push_rr(gb, gb->cpu.bc.val);                            break;
    case 0xc6: add(gb, sm83_fetch_byte(gb), 0);                         break;
    case 0xc7: rst_n(gb, 0x00);                                         break;
    case 0xc8: ret(gb, opcode, gb->cpu.af.flag.z);                    break;
    case 0xc9: ret(gb, opcode, 1);                                      break;
    case 0xca: jp(gb, sm83_fetch_word(gb), 0, gb->cpu.af.flag.z);     break;
    case 0xcb: gb->executed_cycle = execute_cb_instructions(gb, sm83_fetch_byte(gb));        break;
    case 0xcc: call(gb, sm83_fetch_word(gb), gb->cpu.af.flag.z);      break;
    case 0xcd: call(gb, sm83_fetch_word(gb), 1);                        break;
    case 0xce: add(gb, sm83_fetch_byte(gb), gb->cpu.af.flag.c);       break;
    case 0xcf: rst_n(gb, 0x08);                                         break;
    case 0xd0: ret(gb, opcode, !gb->cpu.af.flag.c);                   break;
    case 0xd1: gb->cpu.de.val = sm83_pop_word(gb);                     break;
    case 0xd2: jp(gb, sm83_fetch_word(gb), 0, !gb->cpu.af.flag.c);    break;
    case 0xd4: call(gb, sm83_fetch_word(gb), !gb->cpu.af.flag.c);     break;
    case 0xd5: push_rr(gb, gb->cpu.de.val);                            break;
    case 0xd6: sub(gb, sm83_fetch_byte(gb), 0);                         break;
    case 0xd7: rst_n(gb, 0x10);                                         break;
    case 0xd8: ret(gb, opcode, gb->cpu.af.flag.c);                    break;
    case 0xd9: reti(gb);                                                break;
    case 0xda: jp(gb, sm83_fetch_word(gb), 0, gb->cpu.af.flag.c);     break;
    case 0xdc: call(gb, sm83_fetch_word(gb), gb->cpu.af.flag.c);      break;
    case 0xde: sub(gb, sm83_fetch_byte(gb), gb->cpu.af.flag.c);       break;
    case 0xdf: rst_n(gb, 0x18);                                         break;
    case 0xe0: ldh_indirect_n_a(gb, sm83_fetch_byte(gb));               break;
    case 0xe1: gb->cpu.hl.val = sm83_pop_word(gb);                     break;
    case 0xe2: ldh_indirect_c_a(gb);                                    break;
    case 0xe5: push_rr(gb, gb->cpu.hl.val);                            break;
    case 0xe6: and(gb, sm83_fetch_byte(gb));                            break;
    case 0xe7: rst_n(gb, 0x20);                                         break;
    case 0xe8: add_sp_i8(gb, sm83_fetch_byte(gb));                      break;
    case 0xe9: gb->cpu.pc = gb->cpu.hl.val;                       break;
    case 0xea: bus_write(gb, sm83_fetch_word(gb), gb->cpu.af.a);      break;
    case 0xee: xor(gb, sm83_fetch_byte(gb));                            break;
    case 0xef: rst_n(gb, 0x28);                                         break;
    case 0xf0: ldh_a_indirect_n(gb, sm83_fetch_byte(gb));               break;
    case 0xf1: gb->cpu.af.val = (sm83_pop_word(gb) & 0xfff0) & ~0x000f; break;
    case 0xf2: ldh_a_indirect_c(gb);                                    break;
    case 0xf3: di(gb);                                                  break;
    case 0xf5: push_rr(gb, gb->cpu.af.val);                            break;
    case 0xf6: or(gb, sm83_fetch_byte(gb));                             break;
    case 0xf7: rst_n(gb, 0x30);                                         break;
    case 0xf8: ld_hl_sp_plus_i8(gb, sm83_fetch_byte(gb));               break;
    case 0xf9: gb->cpu.sp = gb->cpu.hl.val;         break;
    case 0xfa: gb->cpu.af.a = bus_read(gb, sm83_fetch_word(gb));      break;
    case 0xfb: ei(gb);                                                  break;
    case 0xfe: cp(gb, sm83_fetch_byte(gb));                             break;
    case 0xff: rst_n(gb, 0x38);                                         break;
    default:
        fprintf(stderr, "Unknown opcode 0x%02x\n", opcode);
        break;
    }
    gb->executed_cycle += interrupt_process(gb);
    return gb->executed_cycle;
}