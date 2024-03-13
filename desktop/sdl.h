#pragma once

#include "gb.h"
#include "joypad.h"
#include "mbc.h"
#include "apu.h"
#include <SDL2/SDL.h>

struct sdl {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    SDL_AudioDeviceID audio_dev;
    SDL_AudioSpec desired_spec, obtained_spec;
    int16_t audio_sample[BUFFER_SIZE];
};

void sdl_init(struct sdl *sdl, int scaler);
void sdl_render(struct sdl *sdl, struct gb *gb);
void sdl_handle_input(struct sdl *sdl, struct gb *gb, bool *done);
void sdl_destroy(struct sdl *sdl);