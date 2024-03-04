/*-*****************************************************************************

MMBasic for Linux (MMB4L)

graphics.c

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

#include <stdbool.h>

#include <SDL.h>

#include "../Configuration.h"
#include "mmresult.h"

#define HRes graphics_current->width
#define VRes graphics_current->height
#define CurrentX graphics_current->x
#define CurrentY graphics_current->y
#define OptionBase mmb_options.base

#define WINDOW_MAX_ID      255
#define WINDOW_MAX_X       2048
#define WINDOW_MAX_Y       2048
#define WINDOW_MAX_WIDTH   2048
#define WINDOW_MAX_HEIGHT  2048

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
#define RGB_NOTBLACK  (VideoColour==8? RGB(0,32, 0,15): (VideoColour==12? RGB(16,16,16,15): (VideoColour==16 ? RGB(0,4,0,15) : RGB(0,0,0,255))))

typedef struct {
    bool dirty;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;
    uint32_t height;
    uint32_t width;
    uint32_t x;
    uint32_t y;
    uint32_t fcolour;
    uint32_t bcolour;
} MmBasicWindow;

extern MmBasicWindow* graphics_current;

/** Terminate graphics sub-system, close all windows, free all resources. */
void graphics_term();

const char* graphics_last_error();
void graphics_pump_events();
MmResult graphics_window_create(int id, int x, int y, int width, int height);
MmResult graphics_window_destroy(int id);
MmResult graphics_window_use(int id);

MmResult graphics_cls();

MmResult graphics_draw_aa_line(MMFLOAT x0 , MMFLOAT y0 , MMFLOAT x1 , MMFLOAT y1, uint32_t c, int w);

/**
 * Draws a box.
 *
 * @param  x1, y1  start coordinates.
 * @param  x2, y2  end coordinates.
 * @param  w       the width of the sides of the box (can be zero).
 * @param  c       colour to use for sides of the box.
 * @param  fill    colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_box(int x1, int y1, int x2, int y2, int w, int c, int fill);

/**
 * Draws a circle or elipse.
 *
 * @param  x, y    coordinates of the center of the circle.
 * @param  radius  radius of the circle.
 * @param  w       width of the line drawing the circle.
 * @param  c       colour to use for the circle.
 * @param  fill    colour to use for the fill or -1 if no fill.
 * @param  aspect  ratio of the x and y axis (a MMFLOAT). 1.0 gives a prefect circle.
 */
MmResult graphics_draw_circle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect);

MmResult graphics_draw_line(int x1, int y1, int x2, int y2, int w, int c);
MmResult graphics_draw_pixel(int x, int y, int c);
MmResult graphics_draw_polygon(unsigned char *p, int close);

/**
 * Draws a box with rounded corners.
 *
 * @param  x1, y1  start coordinates.
 * @param  x2, y2  end coordinates.
 * @param  radius  the radius (in pixels) of the arc forming the corners.
 * @param  c       colour to use for sides of the box.
 * @param  fill    colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_rbox(int x1, int y1, int x2, int y2, int radius, int c, int fill);

MmResult graphics_draw_rectangle(int x1, int y1, int x2, int y2, int c);
