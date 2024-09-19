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

/** The current graphics mode, for CMM2, PicoMiteVGA and MMB4W. */
extern unsigned graphics_mode;

/** Initialises 'graphics' module. */
MmResult graphics_init();

/** Gets the last error message reported by the underlying SDL implementation. */
const char* graphics_last_error();
MmSurfaceId graphics_find_window(uint32_t window_id);

/** Redraws all 'dirty' windows (if the time is right). */
void graphics_refresh_windows();

/** Terminates 'graphics' module, closes all windows, frees all resources. */
MmResult graphics_term();

/** Creates a graphics "blit" buffer. */
MmResult graphics_buffer_create(MmSurfaceId id, int width, int height);

/** Creates a sprite. */
MmResult graphics_sprite_create(MmSurfaceId id, int width, int height);

/**
 * Creates a window.
 *
 * @param  id      Window/surface id (1 .. GRAPHICS_MAX_ID).
 * @param  width   Initial window width.
 * @param  height  Initial window height.
 * @param  x       Initial window x-coordinate.
 * @param  y       Initial window y-coordinate.
 * @param  scale   Scale factor for window, this is only a suggestion and will be reduced if
 *                 necessary to fit the window on the display.
 * @param  title   Title for the window, may be NULL to use a default title.
 * @param  interrupt_addr  Address of interrupt routine to call if an SDL window event is caught.
 * @param  show    If true then show the window on creation, otherwise keep it hidden.
 */
MmResult graphics_window_create(MmSurfaceId id, int width, int height, int x, int y, int scale,
                                const char *title, const char *interrupt_addr, bool show);

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
 * Blits rectangle from one surface to another.
 *
 * @param  src_x, src_y   Top left coordinates on the source surface.
 * @param  dst_x, dst_y   Top left coordinates on the destination surface.
 * @param  width          Width of rectangle to blit.
 * @param  height         Height of rectangle to blit.
 * @param  src_surface    The source surface.
 * @param  dst_surface    The destination surface.
 * @param  flags          Bitwise OR of:
 *                          0x01 = mirrored left to right.
 *                          0x02 = mirrored top to bottom.
 *                          0x04 = don't copy transparent pixels.
 * @param  transparent    Transparent colour, -1 for none.
 */
MmResult graphics_blit(int src_x, int src_y, int dst_x, int dst_y, int width, int height,
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
                                         int transparent);

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
                                           int h, int transparent);

/**
 * Clears a graphics surface.
 *
 * @param  surface  The surface to clear.
 * @param  colour   The colout to clear with.
 */
MmResult graphics_cls(MmSurface *surface, MmGraphicsColour colour);

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
 * Draws an arc of a circle of a given colour and width between two radials (defined in degrees).
 *
 * @param  surface  The surface to draw on.
 * @param  x        The x-coordinate of the center of the arc.
 * @param  y        The y-coordinate of the center of the arc.
 * @param  r1       The inner radius of the arc.
 * @param  r2       The outer radius of the arc.
 * @param  arcrad1  The start radial of the arc in degrees.
 * @param  arcrad2  The end radial of the arc in degrees.
 * @param  colour   The colour of the arc.
 */
MmResult graphics_draw_arc(MmSurface *surface, int x, int y, int r1, int r2, int arcrad1,
                           int arcrad2, MmGraphicsColour colour);

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
 * @param  x1, y1  Start coordinates.
 * @param  x2, y2  End coordinates.
 * @param  width   The line width of the sides of the box (can be zero).
 * @param  colour  Colour to use for sides of the box.
 * @param  fill    Colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
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
 * Draws a filled polygon.
 *
 * @param  n   Number of points (vertices + 1).
 * @param  px  X-coordinates, px[0] == px[n - 1].
 * @param  py  Y-coordinates, py[0] == py[n - 1]
 * @param  c   Line colour.
 * @param  f   Fill colour.
 */
MmResult graphics_draw_filled_polygon(MmSurface *surface, int n, float *px, float *py,
                                      MmGraphicsColour c, MmGraphicsColour f);

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
 * Draws multiple connected lines.
 *
 * @param  n      Number of points.
 * @param  px     Array of x-coordinates.
 * @param  py     Array of y-coordinates.
 * @param  c      Colour for the line segments.
 */
MmResult graphics_draw_polyline(MmSurface *surface, int n, float *px, float *py,
                                MmGraphicsColour c);

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
 * @param  x0, y0  Coordinates of the 0th vertex.
 * @param  x1, y1  Coordinates of the 1st vertex.
 * @param  x2, y2  Coordinates of the 2nd vertex.
 * @param  colour  Colour to use for sides of the triangle.
 * @param  fill    Colour to fill the box (-1 for no fill).
 */
MmResult graphics_draw_triangle(MmSurface *surface, int x0, int y0, int x1, int y1, int x2, int y2,
                                MmGraphicsColour colour, MmGraphicsColour fill);

/**
 * Gets the default title to use for a new window.
 *
 * @param      id        Window id (or MODE if currently simulating other MMBasic platform).
 * @param[out] title     Pointer to buffer that on exit will contain the title.
 * @param[in]  title_sz  Size of the buffer.
 */
MmResult graphics_get_default_window_title(MmSurfaceId id, char *title, size_t title_sz);

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
 * @param  surface      Surface to draw the image on.
 * @param  filename     Name of file to load the image from.
 * @param  x, y         Coordinates of top-left corner to start drawing the image from.
 * @param  transparent  TODO
 * @param  force        TODO
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
 * Sets the current graphics mode.
 *
 * @param  mode          The graphics mode.
 * @param  colour_depth  If == 12 then simulate a three layer CMM2 display:
 *                         page/surface 1 -- top
 *                         page/surface 0
 *                         background     -- bottom
 *                       Pixels in the higher layer overwrite those in the lower levels as defined
 *                       by the transparency/alpha levels of the individual pixels.
 * @param  background    Background colour when transparency == true.
 */
MmResult graphics_set_mode(unsigned mode, unsigned colour_depth, MmGraphicsColour background);

/**
 * Gets a human-readable type for a given surface.
 *
 * @param  surface  The surface.
 * @param  out      The human-readable type is returned in this buffer.
 * @param  out_sz   Size of the \p out buffer.
 */
MmResult graphics_type_as_string(MmSurface *surface, char *out, size_t out_sz);

/**
 * Sets the title of a window.
 *
 * @param  window  The window.
 * @param  title   The new title.
 */
MmResult graphics_window_set_title(MmSurface *window, const char *title);

#endif // #if !defined(MMBASIC_GRAPHICS_H)
