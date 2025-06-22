/*-*****************************************************************************

MMBasic for Linux (MMB4L)

features.h

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMB4L_FEATURES_H)
#define MMB4L_FEATURES_H

#include <stdbool.h>

#include "mmresult.h"
#include "graphics.h"
#include "options.h"

typedef enum {
    kGraphicsTypeCmm2,
    kGraphicsTypeMmb4l,
    kGraphicsTypePicomiteHdmi,
    kGraphicsTypePicomiteLcd,
    kGraphicsTypePicomiteVga,
} FeaturesGraphicsType;

typedef enum {
    kGamepadTypeCmm2,
    kGamepadTypeGamemite,
    kGamepadTypeMmb4l,
    kGamepadTypeMmb4w,
    kGamepadTypeNone,
    kGamepadTypePicomiteSnes,
    kGamepadTypePicomiteUsb
} FeaturesGamepadType;

typedef enum {
    kPlayModfileTypeWithBoth,
    kPlayModfileTypeWithInterrupt,
    kPlayModfileTypeWithSampleRate,
} FeaturesPlayModfileParams;

typedef struct {
    const char *name;
    const char *simple_name;
    const char *device;
    const char *platform;
    FeaturesGraphicsType graphics_type;
    FeaturesGamepadType gamepad_type;
    FeaturesPlayModfileParams play_modfile_params;
    bool has_cmd_flash;                      // Supports FLASH
    bool has_cmd_framebuffer;                // Supports FRAMEBUFFER and BLIT FRAMEBUFFER
    bool has_cmd_mode;                       // Supports MODE
    bool has_cmd_page;                       // Supports PAGE
    bool has_fun_keydown;                    // Supports KEYDOWN()
    bool has_mminfo_drive;                   // Supports MM.INFO(DRIVE)
    bool has_mminfo_cpuspeed;                // Supports MM.INFO(CPUSPEED)
    bool has_mminfo_pin;                     // Supports MM.INFO(PIN)
    bool has_mminfo_ps2;                     // Supports MM.INFO(PS2)
    bool has_mminfo_usb;                     // Supports MM.INFO(USB)
    bool has_mminfo_res;                     // Supports MM.INFO(HRES) and MM.INFO(VRES)
    int hres;
    int vres;
    MmGraphicsColour foreground;
    MmGraphicsColour background;
    OptionsConsole console;
} Features;

MmResult features_init(Features *features, OptionsSimulate optionSimulate);

#endif // #if !defined(MMB4L_FEATURES_H)

