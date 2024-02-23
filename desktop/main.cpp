#include "gb.h"
#include "cartridge.h"
#include "sm83.h"
#include "bus.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
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

    sm83_init(gb);
    while ((opt = getopt(argc, argv, "r:b:")) != -1) {
        switch (opt) {
        case 'r':
            gb->cart.cartridge_loaded = true;
            cartridge_load(gb, optarg, NULL);
            break;
        case 'b':
            gb->cart.boot_rom_loaded = true;
            cartridge_load(gb, NULL, optarg);
            break;
        case '?':
        default:
            abort();
        }
    }
    if (gb->cart.cartridge_loaded)
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
    // setup SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // from 2.0.18: enable native IME
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", 
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }

    // create a texture
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                SCREEN_WIDTH, SCREEN_HEIGHT);
    if (texture == nullptr) {
        SDL_Log("Error creating SDL texture!");
        return -1;
    }

    // // setup DearImGui context
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // enable keyboard controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // enable gamepad controls

    // // setup DearImGui style
    // ImGui::StyleColorsDark();

    // // setup platform/renderer backends
    // ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    // ImGui_ImplSDLRenderer2_Init(renderer);
    
    // our state
    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool test = false;
    struct gb gb;
    bool done = false;

    // setup gbda
    gb_init(&gb, argc, argv);

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // ImGui_ImplSDL2_ProcessEvent(&event);
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

        // // start the Dear ImGui frame
        // ImGui_ImplSDLRenderer2_NewFrame();
        // ImGui_ImplSDL2_NewFrame();
        // ImGui::NewFrame();

        // ImGui::Begin("Screen");
        // ImGui::Image((ImTextureID)texture, ImVec2(160, 144));
        // ImGui::End();


        // // rendering
        // ImGui::Render();
        // SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        // SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        // SDL_RenderClear(renderer);
        // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        // SDL_RenderPresent(renderer);
    }

    // // cleanup
    // ImGui_ImplSDLRenderer2_Shutdown();
    // ImGui_ImplSDL2_Shutdown();
    // ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}