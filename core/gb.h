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

typedef enum {
    NORMAL,
    HALT,
    STOP
} gb_mode_t;

struct sm83 {
    bool ime;
    struct registers {
        union {
            uint16_t af;
            struct {
                union {
                    uint8_t f;
                    struct {
                        uint8_t unused_0 : 1;
                        uint8_t unused_1 : 1;
                        uint8_t unused_2 : 1;
                        uint8_t unused_3 : 1;
                        uint8_t flag_c : 1;
                        uint8_t flag_h : 1;
                        uint8_t flag_n : 1;
                        uint8_t flag_z : 1;
                    };
                };
                uint8_t a;
            };
        };

        union {
            uint16_t bc;
            struct {
                uint8_t c;
                uint8_t b;
            };
        };

        union {
            uint16_t de;
            struct {
                uint8_t e;
                uint8_t d;
            };
        };

        union {
            uint16_t hl;
            struct {
                uint8_t l;
                uint8_t h;
            };
        };

        uint16_t sp;
        uint16_t pc;
    } regs;
};

struct cartridge {
    uint8_t rom[1 * MiB];
    bool cartridge_loaded;
    struct info {
        char name[17];
        uint8_t type;
        int rom_size;
        int ram_size;
        // TODO: fill in other missing infos
    } infos;
    uint8_t boot_code[0xff];
    bool boot_rom_loaded;
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

struct gb {
    uint8_t mem[0x10000];
    gb_mode_t mode;
    struct sm83 cpu;
    struct cartridge cart;
    struct interrupt intr;
    struct timer tim;
};

#ifdef __cplusplus
}
#endif