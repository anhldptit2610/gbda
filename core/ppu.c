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

uint32_t get_color_from_palette(struct gb *gb, palette_t palette, uint8_t color_id)
{
    uint8_t *tmp = NULL;

    switch (palette) {
    case BGP:
        tmp = &gb->ppu.bgp;
        break;
    case OBP0:
        tmp = &gb->ppu.obp0;
        break;
    case OBP1:
        tmp = &gb->ppu.obp1;
        break;
    default:
        break;
    }
    return gb_palette[(*tmp >> (color_id * 2)) & 0x03];
}

uint8_t read_vram(struct gb *gb, uint16_t addr)
{
    return gb->vram[addr - 0x8000];
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
        if (!gb->ppu.lcdc.ppu_enable) {
            set_mode(gb, HBLANK);
            gb->ppu.ly = 0;
        }
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
        gb->ppu.stat.lyc_equal_ly = gb->ppu.lyc == gb->ppu.ly;
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
        gb->ppu.window_in_frame = gb->ppu.wy == gb->ppu.ly;
        break;
    case PPU_REG_WX:
        gb->ppu.wx = val;
        break;
    default:
        break;
    }
}

int cmpfunc(const void *a, const void *b)
{
    struct oam_entry *sa = (struct oam_entry *)a;
    struct oam_entry *sb = (struct oam_entry *)b;

    return (sa->x - sb->x);
}

void ppu_oam_scan(struct gb *gb)
{
    if (gb->ppu.ticks == 80) {
        uint8_t sprite_height = (gb->ppu.lcdc.obj_size) ? 16 : 8;
        for (int i = 0; i < 40; i++) {
            if (gb->oam[i * 4 + 1] > 0 && gb->ppu.oam_entry_cnt < 10 &&
                IN_RANGE(gb->ppu.ly, gb->oam[i * 4] - 16, gb->oam[i * 4] - 17 + sprite_height)) {
                gb->ppu.oam_entry[gb->ppu.oam_entry_cnt].y = gb->oam[i * 4];
                gb->ppu.oam_entry[gb->ppu.oam_entry_cnt].x = gb->oam[i * 4 + 1];
                gb->ppu.oam_entry[gb->ppu.oam_entry_cnt].tile_index = gb->oam[i * 4 + 2];
                gb->ppu.oam_entry[gb->ppu.oam_entry_cnt].attributes.val = gb->oam[i * 4 + 3];
                gb->ppu.oam_entry_cnt++;
            }
            if (gb->ppu.oam_entry_cnt == 10)
                break;
        }
        qsort(gb->ppu.oam_entry, gb->ppu.oam_entry_cnt, sizeof(struct oam_entry), cmpfunc);
        set_mode(gb, DRAWING);
    }
}

void ppu_draw_scanline(struct gb *gb)
{
    uint8_t tile_index, sprite_height, color_id_low, color_id_high, color_id,
            offset_x, offset_y, x_pos, y_pos, sprite_color_id;
    bool is_window_pixel;
    uint16_t tile_map_addr, tile_addr;
    static int window_offset = 0;
    pixel_type_t ptype;

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        // deal with bg and window
        sprite_height = (gb->ppu.lcdc.obj_size) ? 16 : 8;
        is_window_pixel = gb->ppu.lcdc.win_enable && gb->ppu.window_in_frame && i + 7 >= gb->ppu.wx;
        gb->ppu.draw_window_this_line = is_window_pixel;
        tile_map_addr = ((is_window_pixel) ? gb->ppu.lcdc.win_tile_map : gb->ppu.lcdc.bg_tile_map) ? 0x9c00 : 0x9800;
        offset_x = (!is_window_pixel) ? (i + gb->ppu.scx) & 0xff : window_offset;
        offset_y = (!is_window_pixel) ? (gb->ppu.ly + gb->ppu.scy) & 0xff : gb->ppu.window_line_cnt;
        ptype = BG_WIN;
        tile_index = read_vram(gb, tile_map_addr + ((offset_x / 8 + 32 * (offset_y / 8)) & 0x3ff));

        // get tile data
        tile_addr = (gb->ppu.lcdc.bg_win_tiles)
                    ? 0x8000 + 16 * (uint8_t)tile_index : 0x9000 + 16 * (int8_t)tile_index;
        if (is_window_pixel)
            offset_x = window_offset++;
        color_id_low = (read_vram(gb, tile_addr + (offset_y % 8) * 2) >> (7 - (offset_x % 8))) & 0x01;
        color_id_high = (read_vram(gb, tile_addr + (offset_y % 8) * 2 + 1) >> (7 - (offset_x % 8))) & 0x01;
        color_id = color_id_low | (color_id_high << 1);
        gb->ppu.frame_buffer[i + gb->ppu.ly * SCREEN_WIDTH] = (gb->ppu.lcdc.bg_win_enable) 
                                                                    ? get_color_from_palette(gb, BGP, color_id) : gb_palette[0];
        
        // deal with sprite
        if (!gb->ppu.lcdc.obj_enable)
            continue;
        for (int j = gb->ppu.oam_entry_cnt - 1; j >= 0; j--) {
            if (!IN_RANGE(i, gb->ppu.oam_entry[j].x - 8, gb->ppu.oam_entry[j].x))
                continue;
            tile_index = gb->ppu.oam_entry[j].tile_index;
            x_pos = i - (gb->ppu.oam_entry[j].x - 8);
            y_pos = (gb->ppu.ly - (gb->ppu.oam_entry[j].y - 16)) % 16;
            offset_x = (gb->ppu.oam_entry[j].attributes.x_flip) ? x_pos : 7 - x_pos;
            offset_y = (!gb->ppu.oam_entry[j].attributes.y_flip) ? y_pos : sprite_height - 1 - (y_pos);
            if (sprite_height == 16 && y_pos >= 8)  // bottom
                tile_index = (gb->ppu.oam_entry[j].attributes.y_flip) ?  tile_index & 0xfe : tile_index | 0x01;
            else if (sprite_height == 16 && y_pos <= 7) // top
                tile_index = (gb->ppu.oam_entry[j].attributes.y_flip) ?  tile_index | 0x01 : tile_index & 0xfe;
            tile_addr = 0x8000 + 16 * (uint8_t)tile_index;
            // TODO
            uint8_t test = (!gb->ppu.oam_entry[j].attributes.y_flip) ? (y_pos % 8) : 7 - (y_pos % 8);
            // color_id_low = (read_vram(gb, tile_addr + (offset_y) * 2) >> (offset_x)) & 0x01;
            // color_id_high = (read_vram(gb, tile_addr + (offset_y) * 2 + 1) >> (offset_x)) & 0x01;
            color_id_low = (read_vram(gb, tile_addr + (test) * 2) >> (offset_x)) & 0x01;
            color_id_high = (read_vram(gb, tile_addr + (test) * 2 + 1) >> (offset_x)) & 0x01;
            sprite_color_id = color_id_low | (color_id_high << 1);
            if (((ptype == BG_WIN) && (!sprite_color_id || (sprite_color_id > 0 && gb->ppu.oam_entry[j].attributes.priority && color_id > 0))) ||
                ((ptype == SPRITE) && (color_id > 0 && !sprite_color_id)))
                continue;
            gb->ppu.frame_buffer[i + gb->ppu.ly * SCREEN_WIDTH] = 
                            get_color_from_palette(gb, gb->ppu.oam_entry[j].attributes.dmg_palette, sprite_color_id);
            color_id = sprite_color_id;
            ptype = SPRITE;
        }
    }
    window_offset = 0;
}

void ppu_draw(struct gb *gb)
{
    if (gb->ppu.ticks == 252) {
        ppu_draw_scanline(gb);
        set_mode(gb, HBLANK);
    }
}

void ppu_check_stat_intr(struct gb *gb)
{
    bool stat_intr_line = 0;

    if ((gb->ppu.stat.mode0_int_select && gb->ppu.stat.ppu_mode == HBLANK) ||
        (gb->ppu.stat.mode1_int_select && gb->ppu.stat.ppu_mode == VBLANK) ||
        (gb->ppu.stat.mode2_int_select && gb->ppu.stat.ppu_mode == OAM_SCAN) || 
        (gb->ppu.stat.lyc_int_select && gb->ppu.stat.lyc_equal_ly))
        stat_intr_line = 1;
    if (!gb->ppu.stat_intr_line && stat_intr_line)
        interrupt_request(gb, INTR_SRC_LCD);
    gb->ppu.stat_intr_line = stat_intr_line;
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
            gb->ppu.stat_intr_src.val = 0;
            gb->ppu.stat.lyc_equal_ly = gb->ppu.ly == gb->ppu.lyc;
            if (gb->ppu.ly == 144) {
                set_mode(gb, VBLANK);
                interrupt_request(gb, INTR_SRC_VBLANK);
                gb->ppu.frame_ready = true;
                gb->ppu.window_line_cnt = 0;
                gb->ppu.draw_window_this_line = false;
                gb->ppu.window_in_frame = false;
            } else {
                if (gb->ppu.wy == gb->ppu.ly && !gb->ppu.window_in_frame)
                    gb->ppu.window_in_frame = true;
                set_mode(gb, OAM_SCAN);
                if (gb->ppu.draw_window_this_line) {
                    gb->ppu.window_line_cnt++;
                    gb->ppu.draw_window_this_line = false;
                }
            }
            gb->ppu.ticks = 0;
            gb->ppu.oam_entry_cnt = 0;
        }
        break;
    case VBLANK:
        if (gb->ppu.ticks == 456) {
            if (gb->ppu.ly == 153) {
                gb->ppu.ly = 0;
                if (gb->ppu.wy == gb->ppu.ly && !gb->ppu.window_in_frame)
                    gb->ppu.window_in_frame = true;
                gb->ppu.oam_entry_cnt = 0;
                set_mode(gb, OAM_SCAN);
                gb->ppu.stat_intr_src.val = 0;
            } else {
                gb->ppu.ly++;
            }
            gb->ppu.stat.lyc_equal_ly = gb->ppu.ly == gb->ppu.lyc;
            gb->ppu.ticks = 0;
        }
        break;
    default:
        break;
    }
    ppu_check_stat_intr(gb);
}