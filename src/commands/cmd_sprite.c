/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_sprite.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "../common/error.h"
#include "../common/mmb4l.h"
#include "../common/sprite.h"

void cmd_blit_framebuffer(const char *p);
void cmd_blit_scroll(const char *p);
void cmd_blit_write(const char *p);

/** SPRITE LOAD filename$ [, start_sprite] [, colour mode] */
static MmResult cmd_sprite_load(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 1 && argc !=3 && argc != 5) return kArgumentCount;

    const char *filename = getCstring(argv[0]);
    uint8_t start_sprite_id = 0;
    if (mmb_options.simulate == kSimulateMmb4l) {
        if (argc >= 3) start_sprite_id = getint(argv[2], 0, GRAPHICS_MAX_ID);
    } else {
        start_sprite_id = 1;
        if (argc >= 3) start_sprite_id = getint(argv[2], 1, CMM2_SPRITE_COUNT);
        start_sprite_id += CMM2_SPRITE_BASE;
    }
    uint8_t colour_mode = (argc >= 5) ? getint(argv[4], 0, 1) : 0;
    return graphics_load_sprite(filename, start_sprite_id, colour_mode);
}

/** SPRITE READ [#]b, x, y, w, h [, read_surface] */
static MmResult cmd_sprite_read(const char *p) {
    getargs(&p, 11, ",");
    if (argc != 9 && argc != 11) return kArgumentCount;

    if (*argv[0] == '#') argv[0]++;
    MmSurfaceId write_id = -1;
    if (mmb_options.simulate == kSimulateMmb4l) {
        write_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    } else {
        write_id = getint(argv[0], 1, CMM2_SPRITE_COUNT);
        write_id += CMM2_SPRITE_BASE;
    }
    MmSurface *write_surface = &graphics_surfaces[write_id];

    int32_t x = getint(argv[2], 0, WINDOW_MAX_X);
    int32_t y = getint(argv[4], 0, WINDOW_MAX_Y);
    uint32_t w = getint(argv[6], 0, WINDOW_MAX_WIDTH);
    uint32_t h = getint(argv[8], 0, WINDOW_MAX_HEIGHT);

    MmSurfaceId read_id = (argc == 11) ? getint(argv[10], 0, GRAPHICS_MAX_ID) : -1;
    MmSurface *read_surface = (read_id == -1) ? graphics_current : &graphics_surfaces[read_id];
    if (read_surface->type == kGraphicsNone) {
        return error_throw_ex(kGraphicsSurfaceNotFound, "Read surface does not exist");
    }

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > read_surface->width) w = read_surface->width - x;
    if (y + w > read_surface->height) h = read_surface->height - y;
    if (w < 1 || h < 1 || x < 0 || x + w > read_surface->width || y < 0
            || y + h > read_surface->height) return kOk;

    switch (write_surface->type) {
        case kGraphicsNone: {
            MmResult result = graphics_sprite_create(write_id, w, h);
            if (FAILED(result)) return result;
            break;
        }

        case kGraphicsSprite: {
            if (write_surface->width != w || write_surface->height != h) {
                return error_throw_ex(kError, "Existing sprite is incorrect size");
            }
            break;
        }

        case kGraphicsBuffer:
        case kGraphicsWindow: {
            return error_throw_ex(kError, "Existing surface is not a sprite");
        }

        default: {
            return kInternalFault;
        }
    }

    //printf("Created sprite %d\n", write_id);
    return graphics_blit(x, y, 0, 0, w, h, read_surface, write_surface, 0x0, RGB_BLACK);
}

/** SPRITE SET TRANSPARENT rgb121_colour */
static MmResult cmd_sprite_set_transparent(const char *p) {
    uint8_t rgb121_colour = getint(p, 0, 15);
    return sprite_set_transparent_colour(GRAPHICS_RGB121_COLOURS[rgb121_colour]);
}

/**
 * SPRITE SHOW [#]n, x, y, layer [, mode]
 *
 * 'mode' is a bitmask:
 *   - bit 0 set - mirrored left to right.
 *   - bit 1 set - mirrored top to bottom.
 *   - bit 2 set - black pixels not treated as transparent default is 0.
 */
static MmResult cmd_sprite_show(const char *p) {
    if (!graphics_current) {
        return error_throw_ex(kGraphicsSurfaceNotFound, "Write surface not found");
    }

    getargs(&p, 9, ",");
    if (argc != 7 && argc != 9) return kArgumentCount;

    // if (hideall)error((char *)"Sprites are hidden");

    if (*argv[0] == '#') argv[0]++;
    MmSurfaceId surface_id = 0;
    if (mmb_options.simulate == kSimulateMmb4l) {
        surface_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    } else {
        surface_id = getint(argv[0], 1, CMM2_SPRITE_COUNT);
        surface_id += CMM2_SPRITE_BASE;
    }

    if (!graphics_surface_exists(surface_id)) {  // Sprite does not exist.
        return error_throw_ex(kGraphicsSurfaceNotFound, "Sprite not found");
    }

    MmSurface *sprite = &graphics_surfaces[surface_id];
    int32_t x = getint(argv[2], -sprite->width + 1, graphics_current->width - 1);
    int32_t y = getint(argv[4], -sprite->height + 1, graphics_current->height - 1);
    sprite->layer = getint(argv[6], 0, GRAPHICS_MAX_LAYER);
    /*uint8_t mode = (argc == 9) ? getint(argv[8], 0, 7) : 1;*/

    //printf("x = %d, y = %d, layer = %d, mode = %d\n", x, y, layer, mode);
/*
    int layer, mode=1;
    bnbr = (int)getint(argv[0], 1, MAXBLITBUF);									// get the number
    // if(spritebuff[bnbr].h==9999)error("Invalid buffer");
    if (spritebuff[bnbr].spritebuffptr != NULL) {
        x1 = (int)getint(argv[2], -spritebuff[bnbr].w + 1, maxW - 1);
        y1 = (int)getint(argv[4], -spritebuff[bnbr].h + 1, maxH - 1);
        layer = (int)getint(argv[6], 0, MAXLAYER);
        if (argc == 9)spritebuff[bnbr].rotation = (int)getint(argv[8], 0, 7);
        else spritebuff[bnbr].rotation = 0;
        if(spritebuff[bnbr].rotation>3){
            mode |=8;
            spritebuff[bnbr].rotation&=3;
        }
        w = spritebuff[bnbr].w;
        h = spritebuff[bnbr].h;
        if (spritebuff[bnbr].active) {
            layer_in_use[spritebuff[bnbr].layer]--;
            if (spritebuff[bnbr].layer == 0)zeroLIFOremove(bnbr);
            else LIFOremove(bnbr);
            sprites_in_use--;
        }
        spritebuff[bnbr].layer = layer;
        layer_in_use[spritebuff[bnbr].layer]++;
        if (spritebuff[bnbr].layer == 0) zeroLIFOadd(bnbr);
        else LIFOadd(bnbr);
        sprites_in_use++;
//        int cursorhidden = 0;
*/
    //BlitShowBuffer(bnbr, x1, y1, mode);
    MmResult result = sprite_show(sprite, graphics_current, x, y, 0x1);
    if (SUCCEEDED(result)) {
        result = sprite_update_collisions(sprite);
    }
    return result;

/*
        ProcessCollisions(bnbr);
        if (sprites_in_use != LIFOpointer + zeroLIFOpointer || sprites_in_use != sumlayer())error((char *)"sprite internal error");
    }
    else error((char *)"Buffer not in use");
*/
}

void cmd_sprite(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "LOAD"))) {
        result = cmd_sprite_load(p);
    } else if ((p = checkstring(cmdline, "FRAMEBUFFER"))) {
        cmd_blit_framebuffer(p);
    } else if ((p = checkstring(cmdline, "READ"))) {
        result = cmd_sprite_read(p);
    } else if ((p = checkstring(cmdline, "SCROLL"))) {
        cmd_blit_scroll(p);
    } else if ((p = checkstring(cmdline, "SET TRANSPARENT"))) {
        result = cmd_sprite_set_transparent(p);
    } else if ((p = checkstring(cmdline, "SHOW"))) {
        result = cmd_sprite_show(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        cmd_blit_write(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("SPRITE");
    }

    ERROR_ON_FAILURE(result);
}
