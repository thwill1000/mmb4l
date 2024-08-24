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
#include "options.h"
#include "utility.h"
#include "../Configuration.h"

#include <stdbool.h>

#define GRAPHICS_NONE            -1
#define GRAPHICS_MAX_SURFACES    256
#define GRAPHICS_MAX_ID          (GRAPHICS_MAX_SURFACES - 1)
#define WINDOW_MAX_X             2048
#define WINDOW_MAX_Y             2048
#define WINDOW_MAX_WIDTH         2048
#define WINDOW_MAX_HEIGHT        2048
#define WINDOW_MAX_SCALE         10
#define GRAPHICS_SURFACE_N       1
#define GRAPHICS_SURFACE_F       2
#define GRAPHICS_SURFACE_L       3
#define GRAPHICS_MAX_LAYER       4
#define GRAPHICS_MAX_COLLISIONS  4
#define MIN_CMM2_MODE            1
#define MAX_CMM2_MODE            17
#define MIN_PMVGA_MODE           1
#define MAX_PMVGA_MODE           2

#define RGB(red, green, blue, trans) (uint32_t) (((trans & 0xFF) << 24) | ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF))

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

// Additional 4-bit colours defined on the PicoMite.
#define RGB_MYRTLE        RGB(   0, 0x40,    0, 0xFF)
#define RGB_COBALT        RGB(   0, 0x40, 0xFF, 0xFF)
#define RGB_MIDGREEN      RGB(   0, 0x80,    0, 0xFF)
#define RGB_CERULEAN      RGB(   0, 0x80, 0xFF, 0xFF)
#define RGB_MAGENTA_4BIT  RGB(0xFF,    0, 0xFF, 0xFF)
#define RGB_RUST          RGB(0xFF, 0x40,    0, 0xFF)
#define RGB_FUCHSIA       RGB(0xFF, 0x40, 0xFF, 0xFF)
#define RGB_BROWN_4BIT    RGB(0xFF, 0x80,    0, 0xFF)
#define RGB_LILAC         RGB(0xFF, 0x80, 0xFF, 0xFF)

#define CMM2_BLIT_BASE   63
#define CMM2_BLIT_COUNT  64

#define CMM2_SPRITE_BASE  127
#define CMM2_SPRITE_COUNT  64

#define GRAPHICS_OFF_SCREEN  10000

typedef enum {
   kGraphicsFlagSurfaceExists = 0x01,
} GraphicsFlags;

typedef enum {
    kGraphicsNone = 0,
    kGraphicsBuffer,
    kGraphicsSprite,
    kGraphicsInactiveSprite,
    kGraphicsWindow
} GraphicsSurfaceType;

typedef enum {
    kAlignLeft = 0,
    kAlignCenter,
    kAlignRight
} TextHAlign;

typedef enum {
    kAlignTop = 0,
    kAlignMiddle,
    kAlignBottom
} TextVAlign;

typedef enum {
    kBlitNormal = 0x0,
    kBlitHorizontalFlip = 0x1,
    kBlitVerticalFlip = 0x2,
    kBlitWithTransparency = 0x4
} GraphicsBlitType;

typedef enum {
    kOrientNormal = 0,
    kOrientVert,
    kOrientInverted,
    kOrientCounterClock,
    kOrientClockwise
} TextOrientation;

typedef int32_t MmSurfaceId;
typedef int64_t MmGraphicsColour; // 32-bit colour, -1 for transparent background colour.
typedef void* MmWindowPtr;
typedef void* MmRendererPtr;
typedef void* MmTexturePtr;

typedef struct MmSurfaceStruct {
    MmSurfaceId id; 
    GraphicsSurfaceType type;
    bool dirty;
    MmWindowPtr window;
    MmRendererPtr renderer;
    MmTexturePtr texture;
    int height;
    int width;
    uint32_t *pixels;
    const char *interrupt_addr;

    /**
     * Only used for PicoMiteVGA support, it is the colour of the LAYER buffer that should be
     * treated as transparent when blitting/merging it onto the display window.
     */
    MmGraphicsColour transparent;

    // The following fields are only used for type == kGraphicsSprite | kGraphicsInactiveSprite.
    uint32_t *background;
    int x;
    int y;
    int next_x;
    int next_y; 
    uint8_t layer;

    /** Is the sprite collided with the surface/screen edge? */
    uint8_t edge_collisions;

    /** Is the sprite collided with another sprite? */
    int sprite_collisions[32 / sizeof(int)]; // 256 bits.
} MmSurface;

extern const MmGraphicsColour GRAPHICS_RGB121_COLOURS[];

extern MmSurface graphics_surfaces[];
extern MmSurface *graphics_current;
extern MmGraphicsColour graphics_fcolour;
extern MmGraphicsColour graphics_bcolour;
extern uint32_t graphics_font;

MmResult graphics_init();
const char* graphics_last_error();
MmSurfaceId graphics_find_window(uint32_t window_id);

/** Redraws all 'dirty' windows (if the time is right). */
void graphics_refresh_windows();

/** Terminates graphics sub-system, closes all windows, frees all resources. */
MmResult graphics_term();

MmResult graphics_buffer_create(MmSurfaceId id, int width, int height);
MmResult graphics_sprite_create(MmSurfaceId id, int width, int height);
MmResult graphics_window_create(MmSurfaceId id, int x, int y, int width, int height, int scale,
                                const char *title, const char *interrupt_addr);
MmResult graphics_surface_destroy(MmSurface *surface);
MmResult graphics_surface_destroy_all();
MmResult graphics_surface_write(MmSurfaceId id);

static inline bool graphics_surface_exists(MmSurfaceId id) {
    return id >= 0 && id <= GRAPHICS_MAX_ID && graphics_surfaces[id].type != kGraphicsNone;
}

/**
 * Blits rectangle from one surface to another.
 *
 * @param  src_x, src_y   Top left coordinates on the source surface.
 * @param  dst_x, dst_y   Top left coordinates on the destination surface.
 * @param  w, h           Width and height of rectangle.
 * @param  src_surface    The source surface.
 * @param  dst_surface    The destination surface.
 * @param  flags          Bitwise AND of:
 *                          0x01 = mirrored left to right.
 *                          0x02 = mirrored top to bottom.
 *                          0x04 = don't copy transparent pixels.
 * @param  transparent    Transparent colour, -1 for none.
 */
MmResult graphics_blit(int src_x, int src_y, int dst_x, int dst_y, int w, int h,
                       MmSurface *src_surface, MmSurface *dst_surface, unsigned flags,
                       MmGraphicsColour transparent);

/**
 * Blits 4-bit colour compressed "sprite" from memory.
 *
 * @param  surface      The surface to write to.
 * @param  data         Sprite data is read from here.
 * @param  x, y         Top left coordinates on the write surface.
 * @param  w, h         Width and height of sprite.
 * @param  transparent  4-bit colour to treat as transparent, -1 for no transparency.
 */
MmResult graphics_blit_memory_compressed(MmSurface *surface, char *data, int x, int y, int w, int h,
                                         unsigned transparent);

/**
 * Blits 4-bit colour uncompressed "sprite" from memory.
 *
 * @param  surface      The surface to write to.
 * @param  data         Sprite data is read from here.
 * @param  x, y         Top left coordinates on the write surface.
 * @param  w, h         Width and height of sprite.
 * @param  transparent  4-bit colour to treat as transparent, -1 for no transparency.
 */
MmResult graphics_blit_memory_uncompressed(MmSurface *surface, char *data, int x, int y, int w,
                                           int h, unsigned transparent);

/**
 * Draws an anti-aliased straight line.
 *
 * @param  x1, y1  Coordinates of the start of the line.
 * @param  x2, y2  Coordinates of the end of the line.
 * @param  width   Width of the line; valid for horizontal, vertical AND diagonal lines.
 * @param  colour  Colour of the line.
 */
MmResult graphics_draw_aa_line(MmSurface *surface, MMFLOAT x1, MMFLOAT y1, MMFLOAT x2,
                               MMFLOAT y2, MmGraphicsColour colour, int w);

/**
 * Draws a monochrome bitmap.
 *
 * @param  surface  Surface to draw on.
 * @param  x, y     Top left coordinates to start drawing from.
 * @param  width    Width of bitmap.
 * @param  height   Height of bitmap.
 * @param  scale    Integer scaling to apply.
 * @param  fcolour  Foreground colour.
 * @param  bcolour  Background colour.
 * @param  bitmap   Pointer to the bitmap data.
 */
MmResult graphics_draw_bitmap(MmSurface *surface, int x, int y, int width, int height, int scale,
                              MmGraphicsColour fcolour, MmGraphicsColour bcolour,
                              const unsigned char* bitmap);

/**
 * Draws a box.
 *
 * @param  x1, y1  start coordinates.
 * @param  x2, y2  end coordinates.
 * @param  w       the width of the sides of the box (can be zero).
 * @param  colour  colour to use for sides of the box.
 * @param  fill    colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, int w,
                           MmGraphicsColour colour, MmGraphicsColour fill);

/**
 * Draws a single character of text.
 *
 * @param  surface      Surface to draw on.
 * @param  x, y         Top left coordinates to start drawing from.
 * @param  font         Font.
 * @param  fcolour      Foreground colour.
 * @param  bcolour      Background colour.
 * @param  c            The character to draw.
 * @param  orientation  Orientation / rotation.
 */
MmResult graphics_draw_char(MmSurface *surface,  int *x, int *y, uint32_t font,
                            MmGraphicsColour fcolour, MmGraphicsColour bcolour, char c,
                            TextOrientation orientation);

/**
 * Draws a circle or elipse.
 *
 * @param  x, y    coordinates of the center of the circle.
 * @param  radius  radius of the circle.
 * @param  w       width of the line drawing the circle.
 * @param  colour  colour to use for the circle.
 * @param  fill    colour to use for the fill or -1 if no fill.
 * @param  aspect  ratio of the x and y axis (a MMFLOAT). 1.0 gives a prefect circle.
 */
MmResult graphics_draw_circle(MmSurface *surface, int x, int y, int radius, int w,
                              MmGraphicsColour colour, MmGraphicsColour fill, MMFLOAT aspect);

/**
 * Draws a straight line.
 *
 * @param  x1, y1  Coordinates of the start of the line.
 * @param  x2, y2  Coordinates of the end of the line.
 * @param  width   Width of the line; currently only for horizontal and vertical lines.
 * @param  colour  Colour of the line.
 */
MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, int w,
                            MmGraphicsColour colour);

/**
 * Draws a single pixel.
 *
 * @param  x, y    Coordinates of the pixel.
 * @param  colour  Colour of the pixel.
 */
MmResult graphics_draw_pixel(MmSurface *surface, int x, int y, MmGraphicsColour colour);
MmResult graphics_draw_polygon(MmSurface *surface, unsigned char *p, int close);

/**
 * Draws a box with rounded corners.
 *
 * @param  x1, y1  start coordinates.
 * @param  x2, y2  end coordinates.
 * @param  radius  the radius (in pixels) of the arc forming the corners.
 * @param  colour  colour to use for sides of the box.
 * @param  fill    colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_rbox(MmSurface *surface, int x1, int y1, int x2, int y2, int radius,
                            MmGraphicsColour colour, MmGraphicsColour fill);

MmResult graphics_draw_rectangle(MmSurface *surface, int x1, int y1, int x2, int y2,
                                 MmGraphicsColour colour);

/**
 * Draws a string of text.
 *
 * @param  surface  Surface to draw on.
 * @param  x        x-coordinate.
 * @param  y        y-coordinate.
 * @param  font     Font.
 * @param  jh       Horizontal alignment.
 * @param  jv       Vertical alignment.
 * @param  jo       Orientation / rotation.
 * @param  fcolour  Foreground colour.
 * @param  bcolour  Background colour.
 * @param  s        The string to draw.
 */
MmResult graphics_draw_string(MmSurface *surface, int x, int y, uint32_t font, TextHAlign jh,
                              TextVAlign jv, TextOrientation jo, MmGraphicsColour fcolour,
                              MmGraphicsColour bcolour, const char *s);

/**
 * Draws a triangle.
 *
 * @param  x0, y0, x1, y1, x2, y2  coordinates of the vertices.
 * @param  colour  colour to use for sides of the triangle.
 * @param  fill    colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_triangle(MmSurface *surface, int x0, int y0, int x1, int y1, int x2, int y2,
                                MmGraphicsColour colour, MmGraphicsColour fill);

/**
 * Loads a .bmp image.
 *
 * @param  surface   Surface to draw the image on.
 * @param  filename  Name of file to load the image from.
 * @param  x, y      Coordinates of top-left corner to start drawing the image from.
 */
MmResult graphics_load_bmp(MmSurface *surface, char *filename, int x, int y);

/**
 * Loads a .png image.
 *
 * @param  surface   Surface to draw the image on.
 * @param  filename  Name of file to load the image from.
 * @param  x, y      Coordinates of top-left corner to start drawing the image from.
 * @param  transparent
 * @param  force
 */
MmResult graphics_load_png(MmSurface *surface, char *filename, int x, int y, int transparent,
                           int force);

/**
 * Loads a Colour Maximite sprite file.
 *
 * @param  filename      Name of file to load the sprite(s) from.
 * @param  start_sprite  Number to be given for the first sprite.
 * @param  colour_mode   0 to use the original CMM1/CMM2 colour mapping,
 *                       1 to use the PicoMite RGB121 colour mapping.
 */
MmResult graphics_load_sprite(const char *filename, uint8_t start_sprite, uint8_t colour_mode);

/**
 * Scrolls surface.
 *
 * @param  surface  The surface.
 * @param  x        Horizontal scroll increment.
 * @param  y        Vertical scroll increment.
 * @param  fill     Colour to fill space left by pixels that have scrolled off screen.
 */
MmResult graphics_scroll(MmSurface *surface, int x, int y, MmGraphicsColour fill);

/**
 * Sets the default graphics font.
 *
 * @param  font_id  The font Id.
 * @param  scale    Scaling factor 1-15.
 */
MmResult graphics_set_font(uint32_t font_id, int scale);

/**
 * Configures graphics surfaces to simulate a given device/platform/mode.
 *
 * @param  platform      The device/platform to simulate (not kSimulateMmb4l).
 * @param  mode          The graphics mode.
 * @param  colour_depth  If == 12 then simulate a 3 layer CMM2 display:
 *                         page/surface 1 -- top
 *                         page/surface 0
 *                         background     -- bottom
 *                       Pixels in the higher layer overwrite those in the lower levels as defined
 *                       by the transparency/alpha levels of the individual pixels.

 * @param  background    Background colour when transparency == true.
 */
MmResult graphics_simulate_display(OptionsSimulate platform, unsigned mode, unsigned colour_depth,
                                   MmGraphicsColour background);

#endif // #if !defined(MMBASIC_GRAPHICS_H)
