#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define COLOR_WHITE         0x9bbc0fff     /* White */
#define COLOR_LGRAY         0x8bac0fff     /* Light Gray */
#define COLOR_DGRAY         0x306230ff     /* Dark Gray */
#define COLOR_BLACK         0x0f380fff     /* Black */

#define SYSTEM_CLOCK        4194304
#define SCREEN_WIDTH        160
#define SCREEN_HEIGHT       144

#define KiB                 1024
#define MiB                 1048576

#define MSB(n)              (((uint16_t)(n) >> 8) & 0x00ff)
#define LSB(n)              ((uint16_t)(n) & 0x00ff)
#define TO_U16(lsb, msb)    (((uint16_t)(msb) << 8) | (uint16_t)(lsb))
#define BIT(f, n)           (((f) >> (n)) & 0x0001)
#define SET(f, n)           ((f) |= (1U << (n)))
#define RES(f, n)           ((f) &= ~(1U << (n)))
#define IN_RANGE(x, a, b)   ((x) >= (a) && (x) <= (b))

typedef enum {
    NORMAL,
    HALT,
    HALT_BUG,
    STOP
} gb_mode_t;

typedef enum {
    HBLANK,
    VBLANK,
    OAM_SCAN,
    DRAWING,
} ppu_mode_t;

typedef enum {
    OFF,
    WAITING,
    TRANSFERING,
} dma_mode_t;

struct sm83 {
    bool ime;
    union {
        uint16_t val;
        struct {
            union {
                uint8_t f;
                struct {
                    uint8_t unused_0 : 1;
                    uint8_t unused_1 : 1;
                    uint8_t unused_2 : 1;
                    uint8_t unused_3 : 1;
                    uint8_t c : 1;
                    uint8_t h : 1;
                    uint8_t n : 1;
                    uint8_t z : 1;
                } flag;
            };
            uint8_t a;
        };
    } af;

    union {
        uint16_t val;
        struct {
            uint8_t c;
            uint8_t b;
        };
    } bc;

    union {
        uint16_t val;
        struct {
            uint8_t e;
            uint8_t d;
        };
    } de;

    union {
        uint16_t val;
        struct {
            uint8_t l;
            uint8_t h;
        };
    } hl;

    uint16_t sp;
    uint16_t pc;
};

struct cartridge {
    uint8_t rom[1 * MiB];
    bool cartridge_loaded;
    struct info {
        char name[17];
        uint8_t type;
        int rom_size;
        int ram_size;
        int bank_size;
        // TODO: fill in other missing infos
    } infos;
};

typedef enum ACCESS_MODE {
    READ,
    WRITE,
    DUMMY,
} access_mode_t;

struct memory_access_record {
    uint16_t addr;
    uint16_t val;
    access_mode_t mode;
};

struct interrupt {
    uint8_t ie;
    uint8_t flag;
};

struct timer {
    uint16_t div : 14;
    uint8_t tima;
    uint8_t tma;
    union {
        uint8_t val;
        struct {
            uint8_t freq : 2;
            uint8_t enable : 1;
            uint8_t unused : 5;
        };
    } tac;
    bool old_edge;
};

struct oam_entry {
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    union {
        uint8_t val;
        struct {
            uint8_t cgb_palette : 3;
            uint8_t bank : 1;
            uint8_t dmg_palette : 1;
            uint8_t x_flip : 1;
            uint8_t y_flip : 1;
            uint8_t priority : 1;
        };
    } attributes;
};

struct ppu {
    union {
        uint8_t val;
        struct {
            uint8_t bg_win_enable : 1;
            uint8_t obj_enable : 1;
            uint8_t obj_size : 1;
            uint8_t bg_tile_map : 1;
            uint8_t bg_win_tiles : 1;
            uint8_t win_enable : 1;
            uint8_t win_tile_map : 1;
            uint8_t ppu_enable : 1;
        };
    } lcdc;

    union {
        uint8_t val;
        struct {
            uint8_t ppu_mode : 2;
            uint8_t lyc_equal_ly : 1;
            uint8_t mode0_int_select : 1;
            uint8_t mode1_int_select : 1;
            uint8_t mode2_int_select : 1;
            uint8_t lyc_int_select : 1;
            uint8_t unused : 1;
        };
    } stat;

    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;
    uint16_t ticks;
    ppu_mode_t mode;
    uint32_t frame_buffer[SCREEN_HEIGHT * SCREEN_WIDTH];
    bool frame_ready;
    struct oam_entry oam_entry[10];
    uint8_t oam_entry_cnt : 4;
    uint8_t sprite_cnt : 4;
    bool stat_intr_line;
    union {
        uint8_t val;
        struct {
            uint8_t ppu_mode : 2;
            uint8_t lyc_equal_ly : 1;
            uint8_t mode0 : 1;
            uint8_t mode1 : 1;
            uint8_t mode2 : 1;
            uint8_t lyc_int : 1;
            uint8_t unused : 1;
        };
    } stat_intr_src;
    bool window_in_frame;
    int window_line_cnt;
    bool draw_window_this_line;
};

struct dma {
    dma_mode_t mode;
    uint16_t reg;
    uint16_t start_addr;
};

struct joypad {
    union {
        uint8_t val;
        struct {
            uint8_t keys : 4;
            uint8_t select_dpad : 1;
            uint8_t select_button : 1;
            uint8_t unused : 2;
        };
    } joyp;
    bool a;
    bool b;
    bool select;
    bool start;
    bool right;
    bool left;
    bool up;
    bool down;
};

struct mbc {
    struct {
        bool ram_enable;
        uint8_t rom_bank_number : 5;
        uint8_t ram_bank_number : 2;
        bool banking_mode;
    } mbc1;
};

struct gb {
    uint8_t vram[0x2000];
    uint8_t extern_ram[8 * KiB];
    uint8_t wram[0x2000];
    uint8_t echo_ram[0x1e00];
    uint8_t oam[0xa0];
    uint8_t unused[0x60];
    uint8_t hram[0x7f];
    gb_mode_t mode;
    struct sm83 cpu;
    struct cartridge cart;
    struct interrupt intr;
    struct timer tim;
    struct ppu ppu;
    struct dma dma;
    struct joypad joypad;
    struct mbc mbc;
    int screen_scaler;
};

#ifdef __cplusplus
}

#endif