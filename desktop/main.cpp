#include "gb.h"
#include "cartridge.h"
#include "sm83.h"
#include "bus.h"

void run(struct gb *gb, int argc, char *argv[])
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
    while (1) {
        sm83_step(gb);
    }
}

int main(int argc, char *argv[])
{
    struct gb gb;

    run(&gb, argc, argv);
    return 0;
}