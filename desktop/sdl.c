#include "sdl.h"

void sdl_init(struct sdl *sdl, int scaler)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init failed. Error: %s\n", SDL_GetError());
        exit(1);
    }
    sdl->window = SDL_CreateWindow("gbda", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        SCREEN_WIDTH * scaler, SCREEN_HEIGHT * scaler, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!sdl->window) {
        printf("SDL_CreateWindow failed. Error: %s\n", SDL_GetError());
        exit(1);
    }
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!sdl->renderer) {
        printf("SDL_CreateRenderer failed. Error: %s\n", SDL_GetError());
        exit(1);
    }
    sdl->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                    SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!sdl->texture) {
        printf("SDL_CreateTexture failed. Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void sdl_handle_input(struct sdl *sdl, struct gb *gb, bool *done)
{
    SDL_Event event;
    const uint8_t *key_state = NULL;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            *done = true;
    }
    key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_Z] && gb->joypad.b)
        joypad_press_button(gb, JOYPAD_B);
    else if (key_state[SDL_SCANCODE_X] && gb->joypad.a)
        joypad_press_button(gb, JOYPAD_A);
    else if (key_state[SDL_SCANCODE_SPACE] && gb->joypad.select)
        joypad_press_button(gb, JOYPAD_SELECT);
    else if (key_state[SDL_SCANCODE_RETURN] && gb->joypad.start)
        joypad_press_button(gb, JOYPAD_START);
    else if (key_state[SDL_SCANCODE_UP] && gb->joypad.up)
        joypad_press_button(gb, JOYPAD_UP);
    else if (key_state[SDL_SCANCODE_DOWN] && gb->joypad.down)
        joypad_press_button(gb, JOYPAD_DOWN);
    else if (key_state[SDL_SCANCODE_LEFT] && gb->joypad.left)
        joypad_press_button(gb, JOYPAD_LEFT);
    else if (key_state[SDL_SCANCODE_RIGHT] && gb->joypad.right)
        joypad_press_button(gb, JOYPAD_RIGHT);
    else if (!key_state[SDL_SCANCODE_Z] && !gb->joypad.b)
        joypad_release_button(gb, JOYPAD_B);
    else if (!key_state[SDL_SCANCODE_X] && !gb->joypad.a)
        joypad_release_button(gb, JOYPAD_A);
    else if (!key_state[SDL_SCANCODE_SPACE] && !gb->joypad.select)
        joypad_release_button(gb, JOYPAD_SELECT);
    else if (!key_state[SDL_SCANCODE_RETURN] && !gb->joypad.start)
        joypad_release_button(gb, JOYPAD_START);
    else if (!key_state[SDL_SCANCODE_UP] && !gb->joypad.up)
        joypad_release_button(gb, JOYPAD_UP);
    else if (!key_state[SDL_SCANCODE_DOWN] && !gb->joypad.down)
        joypad_release_button(gb, JOYPAD_DOWN);
    else if (!key_state[SDL_SCANCODE_LEFT] && !gb->joypad.left)
        joypad_release_button(gb, JOYPAD_LEFT);
    else if (!key_state[SDL_SCANCODE_RIGHT] && !gb->joypad.right)
        joypad_release_button(gb, JOYPAD_RIGHT);
}

void sdl_render(struct sdl *sdl, struct gb *gb)
{
    SDL_RenderClear(sdl->renderer);
    SDL_UpdateTexture(sdl->texture, NULL, gb->ppu.frame_buffer, SCREEN_WIDTH * 4);
    SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, NULL);
    SDL_RenderPresent(sdl->renderer);
}

void sdl_destroy(struct sdl *sdl)
{
    SDL_DestroyTexture(sdl->texture);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}