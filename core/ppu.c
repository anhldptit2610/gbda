#include "ppu.h"

/* RGBA format */
uint32_t gb_palette[4] = {
    COLOR_WHITE,     /* White */
    COLOR_LGRAY,     /* Light Gray */
    COLOR_DGRAY,     /* Dark Gray */
    COLOR_BLACK,     /* Black */
};

static void set_mode(struct gb *gb, ppu_mode_t mode)
{
    gb->ppu.stat.ppu_mode = mode;
    gb->ppu.mode = mode;
}

uint32_t get_color_from_palette(struct gb *gb, uint8_t code)
{
    return gb_palette[(gb->ppu.bgp >> (code * 2)) & 0x03];
}

uint8_t ppu_read(struct gb *gb, uint16_t addr)
{
    uint8_t ret = 0xff;

    switch (addr) {
    case PPU_REG_LCDC:
        ret = gb->ppu.lcdc.val;
        break;
    case PPU_REG_STAT:
        ret = gb->ppu.stat.val;
        break;
    case PPU_REG_SCY:
        ret = gb->ppu.scy;
        break; 
    case PPU_REG_SCX:
        ret = gb->ppu.scx;
        break; 
    case PPU_REG_LY:
        ret = gb->ppu.ly;
        break; 
    case PPU_REG_LYC:
        ret = gb->ppu.lyc;
        break; 
    case PPU_REG_BGP:
        ret = gb->ppu.bgp;
        break;
    case PPU_REG_OBP0:
        ret = gb->ppu.obp0;
        break;
    case PPU_REG_OBP1:
        ret = gb->ppu.obp1;
        break;
    case PPU_REG_WY:
        ret = gb->ppu.wy;
        break;
    case PPU_REG_WX:
        ret = gb->ppu.wx;
        break;
    default:
        break;
    }
    return ret;
}

void ppu_write(struct gb *gb, uint16_t addr, uint8_t val)
{
    switch (addr) {
    case PPU_REG_LCDC:
        gb->ppu.lcdc.val = val;
        break;
    case PPU_REG_STAT:
        gb->ppu.stat.val = (val & 0xf8) | (gb->ppu.stat.val & 0x87);
        break;
    case PPU_REG_SCY:
        gb->ppu.scy = val;
        break; 
    case PPU_REG_SCX:
        gb->ppu.scx = val;
        break; 
    case PPU_REG_LYC:
        gb->ppu.lyc = val;
        break; 
    case PPU_REG_BGP:
        gb->ppu.bgp = val;
        break;
    case PPU_REG_OBP0:
        gb->ppu.obp0 = val;
        break;
    case PPU_REG_OBP1:
        gb->ppu.obp1 = val;
        break;
    case PPU_REG_WY:
        gb->ppu.wy = val;
        break;
    case PPU_REG_WX:
        gb->ppu.wx = val;
        break;
    default:
        break;
    }
}

void ppu_oam_scan(struct gb *gb)
{
    if (gb->ppu.ticks == 80)
        set_mode(gb, DRAWING);
}


void ppu_draw_scanline(struct gb *gb)
{
    uint8_t tile_index;

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        // read the tile indexes
        uint16_t tile_map_addr = (gb->ppu.lcdc.bg_tile_map) ? 0x9c00 : 0x9800;
        tile_index = gb->mem[tile_map_addr + i / 8 + 32 * ((gb->ppu.ly & 0xff)/ 8)];
        // get tile data
        uint16_t tile_addr = (gb->ppu.lcdc.bg_win_tiles)
                    ? 0x8000 + 16 * (uint8_t)tile_index : 0x9000 + 16 * (int8_t)tile_index;
        uint8_t tile_data_low = (gb->mem[tile_addr + (gb->ppu.ly % 8) * 2]  >> (7 - (i % 8))) & 0x01;
        uint8_t tile_data_high = (gb->mem[tile_addr + (gb->ppu.ly % 8) * 2 + 1]  >> (7 - (i % 8))) & 0x01;
        uint8_t code = tile_data_low | (tile_data_high << 1);
        gb->ppu.frame_buffer[i + gb->ppu.ly * SCREEN_WIDTH] = get_color_from_palette(gb, code);
    }
}

void ppu_draw(struct gb *gb)
{
    if (gb->ppu.ticks == 252) {
        ppu_draw_scanline(gb);
        set_mode(gb, HBLANK);
    }
}

void ppu_tick(struct gb *gb)
{
    gb->ppu.ticks++;
    switch (gb->ppu.mode) {
    case OAM_SCAN:
        ppu_oam_scan(gb);
        break;
    case DRAWING:
        ppu_draw(gb);
        break;
    case HBLANK:
        if (gb->ppu.ticks == 456) {
            gb->ppu.ly++;
            if (gb->ppu.ly == 144) {
                set_mode(gb, VBLANK);
                interrupt_request(gb, INTR_SRC_VBLANK);
                gb->ppu.frame_ready = true;
            } else {
                set_mode(gb, OAM_SCAN);
            }
            gb->ppu.ticks = 0;
        }
        break;
    case VBLANK:
        if (gb->ppu.ticks == 456) {
            if (gb->ppu.ly == 153) {
                gb->ppu.ly = 0;
                set_mode(gb, OAM_SCAN);
            } else {
                gb->ppu.ly++;
            }
            gb->ppu.ticks = 0;
        }
        break;
    default:
        break;
    }

}