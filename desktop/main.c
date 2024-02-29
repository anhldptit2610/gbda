#include "gb.h"
#include "cartridge.h"
#include "sm83.h"
#include "bus.h"
#include "sdl.h"
#include <stdio.h>
#include <unistd.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL2.0.17+ because of SDL_RenderGeometry() function
#endif

void gb_init(struct gb *gb, int argc, char *argv[])
{
    int opt;

    if (argc < 2) {
        fprintf(stderr, "gbda needs at least 2 arguments to work!\n");
        return;
    }

    gb->screen_scaler = 0;
    sm83_init(gb);
    while ((opt = getopt(argc, argv, "r:s:")) != -1) {
        switch (opt) {
        case 'r':
            cartridge_load(gb, optarg);
            gb->cart.cartridge_loaded = true;
            break;
        case 's':
            gb->screen_scaler = atoi(optarg);
            break;
        case '?':
        default:
            abort();
        }
    }
    if (gb->cart.cartridge_loaded)
        load_state_after_booting(gb);
}

int main(int argc, char *argv[])
{
    struct gb gb;
    struct sdl sdl;
    bool done = false;

    gb_init(&gb, argc, argv);
    sdl_init(&sdl, gb.screen_scaler);
    while (!done) {
        sdl_handle_input(&sdl, &gb, &done);
        while (!gb.ppu.frame_ready)
            sm83_step(&gb);
        gb.ppu.frame_ready = false;
        sdl_render(&sdl, &gb);
    }
    return 0;
}