/*-*****************************************************************************

MMBasic for Linux (MMB4L)

image.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#include "cstring.h"
#include "file_private.h"
#include "image.h"
#include "path.h"
#include "streamio.h"
#include "../core/MMBasic.h"  // for perform_background_tasks()
#include "../third_party/spbmp.h"
#include "../third_party/upng.h"

// TODO: duplicated from graphics.c
static inline void graphics_set_pixel_safe(MmSurface *surface, int x, int y, MmGraphicsColour colour) {
    if (x >= 0 && y >= 0 && x < surface->width && y < surface->height) {
        surface->pixels[y*surface->width + x] = colour;
    }
}

static size_t spbmp_file_read_cb(void *file, void *buffer, size_t size, size_t count,
                                 void *userdata) {
    return fread(buffer, size, count, (FILE *) file);
}

static size_t spbmp_file_write_cb(void *file, const void *buffer, size_t size, size_t count,
                                  void *userdata) {
    return fwrite(buffer, size, count, (FILE *) file);
}

static SpColourArgb spbmp_get_pixel_cb(int x, int y, void *userdata) {
    MmGraphicsColour colour = RGB_BLACK;
    assert(graphics_get_pixel((MmSurface *) userdata, x, y, &colour) == kOk);
    return colour >= 0 ? (SpColourArgb) colour : (SpColourArgb) RGB_BLACK;
}

static void spbmp_set_pixel_cb(int x, int y, SpColourArgb colour, void *userdata) {
    graphics_set_pixel_safe((MmSurface *) userdata, x, y, (MmGraphicsColour) colour);
}

static int spbmp_abort_check_cb(void *userdata) {
    perform_background_tasks();
    return 0;
}

MmResult image_load_bmp(MmSurface *surface, char *filename, int x, int y) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;
    char _filename[STRINGSIZE];
    ON_FAILURE_RETURN(path_try_extension(filename, ".bmp", _filename, STRINGSIZE));

    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(_filename, "rb", fnbr));
    spbmp_init(spbmp_file_read_cb, NULL, NULL, spbmp_set_pixel_cb, spbmp_abort_check_cb);
    SpBmpResult bmp_result = spbmp_read(file_table[fnbr].file_ptr, x, y, surface);
    surface->dirty = true;
    ON_FAILURE_LOG(streamio_close(fnbr));
    return SUCCEEDED(bmp_result) ? kOk : kGraphicsLoadBitmapFailed;
}

static void image_draw_buffer(MmSurface *surface, int x1, int y1, int x2, int y2,
                              const unsigned char* buffer, int skip) {
    const unsigned char *psrc = buffer;
    union colourmap
    {
        char rgbbytes[4];
        uint32_t rgb;
    } c;
    int scale = 1; // (PageTable[WritePage].expand ? 2 : 1);
    //if (optiony)y1=maxH-1-y1;
    //if (optiony)y2=maxH-1-y2;
    // make sure the coordinates are kept within the display area
    if (x2 <= x1) SWAP(int, x1, x2);
    if (y2 <= y1) SWAP(int, y1, y2);
    // int cursorhidden=0;
    // if (cursoron)
    //     if ( !(xcursor + wcursor < x1 ||
    //         xcursor > x2 ||
    //         ycursor + hcursor < y1 ||
    //         ycursor > y2)){
    //     hidecursor(0);
    //     cursorhidden=1;
    //     }
    if (scale==1){
        for (int y = y1; y <= y2; y++){
            // routinechecks(1);
            uint32_t *pdst = surface->pixels + (y * surface->width + x1);
            for (int x = x1; x <= x2; x++){
                if (x >= 0 && x < surface->width && y >= 0 && y < surface->height) {
                    if (skip & 2) {
                        c.rgbbytes[3] = 0xFF; //assume solid colour
                        c.rgbbytes[2] = *psrc++; //this order swaps the bytes to match the .BMP file
                        c.rgbbytes[1] = *psrc++;
                        c.rgbbytes[0] = *psrc++;
                        if (skip & 1) c.rgbbytes[3] = *psrc++; //ARGB8888 so set transparency
                    } else {
                        c.rgbbytes[3] = 0;
                        c.rgbbytes[0] = *psrc++; //this order swaps the bytes to match the .BMP file
                        c.rgbbytes[1] = *psrc++;
                        c.rgbbytes[2] = *psrc++;
                        if (skip & 1) psrc++;
                    }
                    *pdst = c.rgb;
                } else {
                    psrc += (skip & 1) ? 4 : 3;
                }
                pdst++;
            }
        }
    }
    // } else {
    //     uint32_t *s1;
    //     for(y=y1*2;y<=y2*2;y+=2){
    //         routinechecks(1);
    //         sc=(uint32_t *)((y * maxW + x1) * 4 + wpa);
    //         s1=(uint32_t *)(((y+1) * maxW + x1) * 4 + wpa);
    //         for(x=x1;x<=x2;x++){
    //             if (x>=0 && x<maxW && y>=0 && y<maxH*2){
    //                 if (skip & 2){
    //                     c.rgbbytes[3]=0xFF;
    //                     c.rgbbytes[2]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[0]=*p++;
    //                     if (skip & 1)c.rgbbytes[3]=*p++; //ARGB8888 so set transparency
    //                 } else {
    //                     c.rgbbytes[3]=0;
    //                     c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
    //                     c.rgbbytes[1]=*p++;
    //                     c.rgbbytes[2]=*p++;
    //                     if (skip & 1)p++;
    //                 }
    //                 *sc=c.rgb;
    //                 *s1=*sc;
    //             } else {
    //                 p+=(skip & 1) ? 4 : 3;
    //             }
    //             sc++;
    //             s1++;
    //         }
    //     }
    // }
    // if (cursorhidden)showcursor(0, xcursor,ycursor);
}

MmResult image_load_png(MmSurface *surface, char *filename, int x, int y, int transparent,
                        int force) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidWriteSurface;
    char _filename[STRINGSIZE];
    MmResult result = path_try_extension(filename, ".png", _filename, STRINGSIZE);
    if (FAILED(result)) return result;

    upng_t *upng = upng_new_from_file(_filename);
    // routinechecks(1);
    upng_header(upng);
    const int w = upng_get_width(upng);
    const int h = upng_get_height(upng);
    if (x + w > graphics_current->width || y + h > graphics_current->height) {
        upng_free(upng);
        return kImageTooLarge;
    }
    if (!(upng_get_format(upng)==1 || upng_get_format(upng)==3)){
        upng_free(upng);
        return kImageInvalidFormat;
    }
    // routinechecks(1);
    upng_decode(upng);
    // routinechecks(1);
    const unsigned char *buffer = upng_get_buffer(upng);
    // int savey = optiony;
    // optiony = 0;
    if (upng_get_format(upng) ==3) {
        image_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 3 | transparent | force);
    } else {
        image_draw_buffer(surface, x, y, x + w - 1, y + h - 1, buffer, 2 | transparent | force);
    }
    // optiony = savey;
    upng_free(upng);
    // clearrepeat();
    surface->dirty = true;
    return kOk;
}

MmResult image_save_bmp(MmSurface *surface, char *filename, BmpFormat format, int x, int y,
                        int width, int height) {
    if (!surface || surface->type == kGraphicsNone) return kGraphicsInvalidReadSurface;
    char _filename[STRINGSIZE];
    if (FAILED(cstring_cpy(_filename, filename, sizeof(_filename)))) return kFilenameTooLong;

    // If the filename does not have a ".bmp" extension then add one.
    if (cstring_casecmp(path_get_extension(_filename), ".bmp") != 0) {
        if (FAILED(cstring_cat(_filename, ".bmp", STRINGSIZE))) return kFilenameTooLong;
    }

    SpBmpFormat spFormat;
    switch (format) {
        case kBmpFormat1bpp:
            spFormat = kSpBmp1bpp;
            break;
        case kBmpFormat4bppRgb121:
            spFormat = kSpBmp4bppRgb121;
            break;
        case kBmpFormat4bppRgb121Rle4:
            spFormat = kSpBmp4bppRgb121Rle4;
            break;
        case kBmpFormat8bppRgb222:
            spFormat = kSpBmp8bppRgb222;
            break;
        case kBmpFormat8bppRgb222Rle8:
            spFormat = kSpBmp8bppRgb222Rle8;
            break;
        case kBmpFormat8bppRgb332:
            spFormat = kSpBmp8bppRgb332;
            break;
        case kBmpFormat8bppRgb332Rle8:
            spFormat = kSpBmp8bppRgb332Rle8;
            break;
        case kBmpFormat16bppRgb555:
            spFormat = kSpBmp16bppRgb555;
            break;
        case kBmpFormat16bppRgb565:
            spFormat = kSpBmp16bppRgb565;
            break;
        case kBmpFormat24bpp:
            spFormat = kSpBmp24bpp;
            break;
        case kBmpFormat32bpp:
            spFormat = kSpBmp32bpp;
            break;
        default:
            return kImageInvalidFormat;
    }

    // Open the file for writing.
    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(_filename, "wb", fnbr));

    // Write the bitmap to the file.
    spbmp_init(NULL, spbmp_file_write_cb, spbmp_get_pixel_cb, NULL, spbmp_abort_check_cb);
    SpBmpResult bmp_result = spbmp_write(file_table[fnbr].file_ptr, spFormat, surface, x, y,
                                         width, height);

    ON_FAILURE_LOG(streamio_close(fnbr));

    return SUCCEEDED(bmp_result) ? kOk : kGraphicsSaveBitmapFailed;
}
