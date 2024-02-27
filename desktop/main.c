#include "gb.h"
#include "cartridge.h"
#include "sm83.h"
#include "bus.h"
#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>

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
    while ((opt = getopt(argc, argv, "r:b:s:")) != -1) {
        switch (opt) {
        case 'r':
            gb->cart.cartridge_loaded = true;
            cartridge_load(gb, optarg, NULL);
            break;
        case 'b':
            gb->cart.boot_rom_loaded = true;
            cartridge_load(gb, NULL, optarg);
            break;
        case 's':
            gb->screen_scaler = atoi(optarg);
            break;
        case '?':
        default:
            abort();
        }
    }
    if (gb->cart.cartridge_loaded && !gb->cart.boot_rom_loaded)
        load_state_after_booting(gb);
}

// int main(int argc, char *argv[])
// {
//     struct gb gb;

//     run(&gb, argc, argv);
//     return 0;
// }

int main(int argc, char *argv[])
{
    struct gb gb;
    bool done = false;

    // setup gbda
    gb_init(&gb, argc, argv);

    // setup SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // from 2.0.18: enable native IME
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("mgba", 
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * gb.screen_scaler, 
                            SCREEN_HEIGHT * gb.screen_scaler, window_flags);
    if (window == NULL) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                SCREEN_WIDTH, SCREEN_HEIGHT);
    if (texture == NULL) {
        SDL_Log("Error creating SDL texture!");
        return -1;
    }

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;
        }

        while (!gb.ppu.frame_ready)
            sm83_step(&gb);
        gb.ppu.frame_ready = false;
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(texture, NULL, gb.ppu.frame_buffer, SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}