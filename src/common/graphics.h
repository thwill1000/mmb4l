/*-*****************************************************************************

MMBasic for Linux (MMB4L)

graphics.h

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

#if !defined(MMBASIC_GRAPHICS_H)
#define MMBASIC_GRAPHICS_H

#include "mmresult.h"
#include "utility.h"
#include "../Configuration.h"

#include <stdbool.h>

#define GRAPHICS_NONE          -1
#define GRAPHICS_MAX_SURFACES  256
#define GRAPHICS_MAX_ID        (GRAPHICS_MAX_SURFACES - 1)
#define WINDOW_MAX_X           2048
#define WINDOW_MAX_Y           2048
#define WINDOW_MAX_WIDTH       2048
#define WINDOW_MAX_HEIGHT      2048
#define WINDOW_MAX_SCALE       10

#define RGB(red, green, blue, trans) (uint32_t) (((trans & 0b1111) << 24) | ((red & 0b11111111) << 16) | ((green  & 0b11111111) << 8) | (blue & 0b11111111))

#define RGB_BLACK     RGB(   0,     0,     0,     0)
#define RGB_BLUE      RGB(   0,     0,   255,   255)
#define RGB_GREEN     RGB(   0,   255,     0,   255)
#define RGB_CYAN      RGB(   0,   255,   255,   255)
#define RGB_RED       RGB( 255,     0,     0,   255)
#define RGB_MAGENTA   RGB( 255,     0,   192,   255)
#define RGB_YELLOW    RGB( 255,   255,     0,   255)
#define RGB_BROWN     RGB(0xA5,  0x2A,  0x2A,   255)
#define RGB_GRAY      RGB(  64,    64,    64,   255)
#define RGB_LITEGRAY  RGB( 128,   128,   128,   255)
#define RGB_WHITE     RGB( 255,   255,   255,   255)
#define RGB_ORANGE    RGB(0xFF,  0xA5,     0,   255)
#define RGB_PINK      RGB(0xFF,  0xA0,  0xAB,   255)
#define RGB_GOLD      RGB(0xFF,  0xD7,  0x00,   255)
#define RGB_SALMON    RGB(0xFA,  0x80,  0x72,   255)
#define RGB_BEIGE     RGB(0xF5,  0xF5,  0xDC,   255)
#define RGB_NOTBLACK  RGB(   0,     0,     0,   255)

typedef enum {
    kGraphicsNone = 0,
    kGraphicsBuffer,
    kGraphicsSprite,
    kGraphicsWindow
} GraphicsSurfaceType;

typedef int32_t MmSurfaceId;
typedef int64_t MmGraphicsColour; // 32-bit colour, -1 for transparent background colour.
typedef void* MmWindowPtr;
typedef void* MmRendererPtr;
typedef void* MmTexturePtr;

typedef struct {
    GraphicsSurfaceType type;
    bool dirty;
    MmWindowPtr window;
    MmRendererPtr renderer;
    MmTexturePtr texture;
    uint32_t* pixels;
    int height;
    int width;
} MmSurface;

extern MmSurface graphics_surfaces[];
extern MmSurface *graphics_current;
extern MmGraphicsColour graphics_fcolour;
extern MmGraphicsColour graphics_bcolour;

/** Initialises 'graphics' module. */
MmResult graphics_init();

/** Gets the last error message reported by the underlying SDL implementation. */
const char* graphics_last_error();

/** Redraws all 'dirty' windows (if the time is right). */
void graphics_refresh_windows();

/** Terminates 'graphics' module, closes all windows, frees all resources. */
MmResult graphics_term();

/** Creates a graphics "blit" buffer. */
MmResult graphics_buffer_create(MmSurfaceId id, int width, int height);

/** Creates a window. */
MmResult graphics_window_create(MmSurfaceId id, int x, int y, int width, int height, int scale);

/** Destroys a graphics surface. */
MmResult graphics_surface_destroy(MmSurface *surface);

/** Destroys all graphics surfaces. */
MmResult graphics_surface_destroy_all();

/** Sets the default write surface. */
MmResult graphics_surface_write(MmSurfaceId id);

/** Checks if a surface exists. */
static inline bool graphics_surface_exists(MmSurfaceId id) {
    return id >= 0 && id <= GRAPHICS_MAX_ID && graphics_surfaces[id].type != kGraphicsNone;
}

/**
 * Draws an anti-aliased straight line.
 *
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  width   Width of the line; valid for horizontal, vertical AND diagonal lines.
 * @param  colour  Colour of the line.
 */
MmResult graphics_draw_aa_line(MmSurface *surface, MMFLOAT x1, MMFLOAT y1, MMFLOAT x2,
                               MMFLOAT y2, int width, MmGraphicsColour colour);

/**
 * Draws a box.
 *
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  width   The line width of the sides of the box (can be zero).
 * @param  colour  Colour to use for sides of the box.
 * @param  fill    Colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, uint32_t width,
                           MmGraphicsColour colour, MmGraphicsColour fill);

/** 
 * Draws a circle or elipse.
 *
 * @param  x, y    Coordinates of the center of the circle.
 * @param  radius  Radius of the circle.
 * @param  width   Width of the line drawing the circle.
 * @param  colour  Colour to use for the circle.
 * @param  fill    Colour to use for the fill or -1 if no fill.
 * @param  aspect  Ratio of the x and y axis (a MMFLOAT). 1.0 gives a prefect circle.
 */
MmResult graphics_draw_circle(MmSurface *surface, int x, int y, int radius, int width,
                              MmGraphicsColour colour, MmGraphicsColour fill, MMFLOAT aspect);

/**
 * Draws a straight line.
 *
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  width   Width of the line; currently only for horizontal and vertical lines.
 * @param  colour  Colour of the line.
 */
MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
                            MmGraphicsColour colour);

/**
 * Draws a single pixel.
 *
 * @param  x, y    Coordinates of the pixel.
 * @param  colour  Colour of the pixel.
 */
MmResult graphics_draw_pixel(MmSurface *surface, int x, int y, MmGraphicsColour colour);

/**
 * Draws a box with rounded corners.
 *
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  radius  The radius (in pixels) of the arc forming the corners.
 * @param  colour  Colour to use for sides of the box.
 * @param  fill    Colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_rbox(MmSurface *surface, int x1, int y1, int x2, int y2, int radius,
                            MmGraphicsColour colour, MmGraphicsColour fill);

/**
 * Draws a filled rectangle.
 *
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  colour  Colour to draw/fill the rectangle.
 */
MmResult graphics_draw_rectangle(MmSurface *surface, int x1, int y1, int x2, int y2,
                                 MmGraphicsColour colour);

#endif // #if !defined(MMBASIC_GRAPHICS_H)
