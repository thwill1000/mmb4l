/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_graphics.c

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

#include <assert.h>
#include <stdio.h>

#include "../common/console.h"
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/graphics.h"
#include "../common/mmb4l.h"
#include "../common/utility.h"
#include "../core/tokentbl.h"

/** GRAPHICS BUFFER id, width, height */
static MmResult cmd_graphics_buffer(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) return kArgumentCount;
    MmSurfaceId id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    int width = getint(argv[2], 8, WINDOW_MAX_WIDTH);
    int height = getint(argv[4], 8, WINDOW_MAX_HEIGHT);

    return graphics_buffer_create(id, width, height);
}

/** GRAPHICS CLS id [, colour] */
static MmResult cmd_graphics_cls(const char *p) {
    getargs(&p, 3, ",");
    if (argc % 2 != 1) return kArgumentCount;
    const MmSurfaceId id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    MmSurface *surface = &graphics_surfaces[id];
    const MmSurface *layer = (mmb_options.simulate == kSimulatePicoMiteVga)
            ?  &graphics_surfaces[GRAPHICS_SURFACE_L]
            : NULL;
    MmGraphicsColour colour = has_arg(2)
            ? getint(argv[2], RGB_BLACK, RGB_WHITE)
            : (surface == layer) ? layer->transparent : graphics_bcolour;

    return graphics_cls(surface, colour);
}

/**
 * GRAPHICS COPY src_id TO dst_id [, when] [, transparent]
 *
 * @param  when         Ignored for the moment by MMB4L.
 * @param  transparent  If T or 1 then treat BLACK as transparent when copying.
 */
MmResult cmd_graphics_copy(const char *p) {
    char ss[3];
    ss[0] = tokenTO;
    ss[1] =',';
    ss[2] = 0;
    getargs(&p, 7, ss);
    if (argc < 3 || !(argc % 2)) return kArgumentCount;

    MmSurfaceId src_id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    if (!graphics_surface_exists(src_id)) return kGraphicsInvalidReadSurface;
    MmSurface* src_surface = &graphics_surfaces[src_id];

    MmSurfaceId dst_id = getint(argv[2], 0, GRAPHICS_MAX_ID);
    if (!graphics_surface_exists(dst_id)) return kGraphicsInvalidWriteSurface;
    MmSurface* dst_surface = &graphics_surfaces[dst_id];

    // if (src_surface->width != dst_surface->width || src_surface->height != dst_surface->height) {
    //     return kGraphicsSurfaceSizeMismatch;
    // }

    if (has_arg(4)) {
        const char *p = argv[4];
        switch (toupper(*p)) {
            case 'I':
            case 'B':
            case 'D':
                // Ignore for now
                break;
            default:
                return kSyntax;
        }
    }

    MmGraphicsColour transparent = -1;
    if (has_arg(6)) {
        const char *p = argv[6];
        if (toupper(*p) == 'T' || *p == '1') transparent = RGB_BLACK;
    }

    return graphics_copy(src_surface, dst_surface, transparent);
}

/** GRAPHICS SETTITLE id, title$ */
static MmResult cmd_graphics_set_title(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) return kArgumentCount;
    const MmSurfaceId id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    const char *title = getCstring(argv[2]);

    return graphics_window_set_title(&graphics_surfaces[id], title);
}

/** GRAPHICS SPRITE id, width, height */
static MmResult cmd_graphics_sprite(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) return kArgumentCount;
    MmSurfaceId sprite_id = -1;
    ON_FAILURE_RETURN(parse_sprite_id(argv[0], 0x0, &sprite_id));
    const int width = getint(argv[2], 8, WINDOW_MAX_WIDTH);
    const int height = getint(argv[4], 8, WINDOW_MAX_HEIGHT);

    return graphics_sprite_create(sprite_id, width, height);
}

/** GRAPHICS WINDOW id, width, height [, x] [, y] [, title$] [, scale] [, interrupt] */
static MmResult cmd_graphics_window(const char *p) {
    getargs(&p, 15, ",");
    if (argc < 5 || argc > 15 || !(argc % 2)) return kArgumentCount;
    const MmSurfaceId id = getint(argv[0], 0, GRAPHICS_MAX_ID);
    const int width = getint(argv[2], 8, WINDOW_MAX_WIDTH);
    const int height = getint(argv[4], 8, WINDOW_MAX_HEIGHT);
    const int x = has_arg(6) ? getint(argv[6], 0, WINDOW_MAX_X) : -1;
    const int y = has_arg(8) ? getint(argv[8], 0, WINDOW_MAX_Y) : -1;
    const char *title = has_arg(10) ? getCstring(argv[10]) : NULL;
    const int scale = has_arg(12) ? getint(argv[12], 1, WINDOW_MAX_SCALE) : 1;
    const char *interrupt_addr = has_arg(14) ? GetIntAddress(argv[12]) : NULL;
    if (interrupt_addr) {
        // Check interrupt is a SUB with the correct signature.
        FunctionSignature *fn = (FunctionSignature *) GetTempMemory(sizeof(FunctionSignature));
        const char *p2 = interrupt_addr;
        const char *cached_line_ptr = CurrentLinePtr;
        CurrentLinePtr = interrupt_addr; // So any error is reported on the correct line.
        MmResult result = parse_fn_sig(&p2, fn);
        if (FAILED(result)) return result;
        CurrentLinePtr = cached_line_ptr;
        if (fn->token != cmdSUB
            || fn->num_params != 2
            || !(fn->params[0].type & T_INT)
            || !(fn->params[1].type & T_INT)
            || fn->params[0].array
            || fn->params[1].array) return kInvalidInterruptSignature;
        ClearSpecificTempMemory(fn);
    }

    // TODO: width & height should be divisible by 8.
    // TODO: check window has not already been created.

    return graphics_window_create(id, width, height, x, y, scale, title, interrupt_addr, true);
}

/** GRAPHICS DESTROY { id | ALL } */
static MmResult cmd_graphics_destroy(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    if ((p = checkstring(argv[0], "ALL"))) {
        return graphics_surface_destroy_all();
    } else {
        MmSurfaceId id = getint(argv[0], 0, GRAPHICS_MAX_ID);
        return graphics_surface_destroy(&graphics_surfaces[id]);
    }
}

/** GRAPHICS LIST */
MmResult cmd_graphics_list(const char *p) {
    if (!parse_is_end(p)) return kArgumentCount;

    MmResult result = kOk;
    char buf[STRINGSIZE];

    if (mmb_options.simulate != kSimulateMmb4l) {
        result = options_get_string_value(&mmb_options, kOptionSimulate, buf);
        if (FAILED(result)) return result;
        console_puts(buf);
        if (mmb_options.simulate == kSimulateGameMite) {
            console_puts("\r\n");
        } else {
            (void) snprintf(buf, STRINGSIZE, " - Mode %d\r\n", graphics_mode);
            console_puts(buf);
        }
    }

    const MmSurfaceId current_id = graphics_current ? graphics_current->id : -1;
    char type[64];
    int count = 0;
    for (MmSurfaceId id  = 0; id <= GRAPHICS_MAX_ID; ++id) {
        MmSurface *s = &graphics_surfaces[id];
        if (s->type == kGraphicsNone) continue;
        result = graphics_type_as_string(s, type, 64);
        if (FAILED(result)) break;
        snprintf(buf, STRINGSIZE, "%c %3d) %s: %d x %d\r\n", id == current_id ? '*' : ' ',
                 id, type, s->width, s->height);
        console_puts(buf);
        count++;
    }
    if (SUCCEEDED(result) && count == 0) {
        console_puts("No graphics surfaces");
    }

    return result;
}

/** GRAPHICS WRITE { id | NONE } */
MmResult cmd_graphics_write(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    if ((p = checkstring(argv[0], "NONE"))) {
        return graphics_surface_write(GRAPHICS_NONE);
    } else {
        assert(GRAPHICS_NONE == -1);
        MmSurfaceId id = getint(argv[0], -1, GRAPHICS_MAX_ID);
        return graphics_surface_write(id);
    }
}

void cmd_graphics(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "BUFFER"))) {
        result = cmd_graphics_buffer(p);
    } else if ((p = checkstring(cmdline, "CLS"))) {
        result = cmd_graphics_cls(p);
    } else if ((p = checkstring(cmdline, "COPY"))) {
        result = cmd_graphics_copy(p);
    } else if ((p = checkstring(cmdline, "DESTROY"))) {
        result = cmd_graphics_destroy(p);
    } else if ((p = checkstring(cmdline, "LIST"))) {
        result = cmd_graphics_list(p);
    } else if ((p = checkstring(cmdline, "SETTITLE"))) {
        result = cmd_graphics_set_title(p);
    } else if ((p = checkstring(cmdline, "SPRITE"))) {
        result = cmd_graphics_sprite(p);
    } else if ((p = checkstring(cmdline, "WINDOW"))) {
        result = cmd_graphics_window(p);
    } else if ((p = checkstring(cmdline, "WRITE"))) {
        result = cmd_graphics_write(p);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("GRAPHICS");
    }
    ERROR_ON_FAILURE(result);
}
