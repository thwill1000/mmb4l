/*-*****************************************************************************

MMBasic for Linux (MMB4L)

features.c

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

#include <string.h>

#include "features.h"
#include "logger.h"

static Features features_cmm2 = {
    .name = "Colour Maximite 2",
    .simple_name = "CMM2",
    .device = "Colour Maximite 2",
    .platform = "",
    .gamepad_type = kGamepadTypeCmm2,
    .graphics_type = kGraphicsTypeCmm2,
    .play_modfile_params = kPlayModfileTypeWithSampleRate,
    .has_cmd_mode = true,
    .has_cmd_page = true,
    .has_fun_keydown = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kBoth
};

static Features features_gamemite = {
    .name = "Game*Mite",
    .simple_name = "GameMite",
    .device = "PicoMite",
    .platform = "Game*Mite",
    .gamepad_type = kGamepadTypeGamemite,
    .graphics_type = kGraphicsTypePicomiteLcd,
    .play_modfile_params = kPlayModfileTypeWithInterrupt,
    .has_cmd_flash = true,
    .has_cmd_framebuffer = true,
    .has_mminfo_cpuspeed = true,
    .has_mminfo_drive = true,
    .has_mminfo_pin = true,
    .hres = 320,
    .vres = 240,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kScreen
};

static Features features_mmb4l = {
    .name = "MMB4L",
    .simple_name = "MMB4L",
    .device = "MMB4L",
    .platform = "",
    .gamepad_type = kGamepadTypeMmb4l,
    .graphics_type = kGraphicsTypeMmb4l,
    .play_modfile_params = kPlayModfileTypeWithBoth,
    .has_mminfo_ps2 = true,
    .has_mminfo_res = true,
    .has_fun_keydown = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kSerial
};

static Features features_mmb4w = {
    .name = "MMBasic for Windows",
    .simple_name = "MMB4W",
    .device = "MMBasic for Windows",
    .platform = "",
    .gamepad_type = kGamepadTypeMmb4w,
    .graphics_type = kGraphicsTypeCmm2,
    .play_modfile_params = kPlayModfileTypeWithSampleRate,
    .has_cmd_mode = true,
    .has_cmd_page = true,
    .has_fun_keydown = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kBoth
};

static Features features_picocalc = {
    .name = "PicoCalc",
    .simple_name = "PicoCalc",
    .device = "PicoMite",
    .platform = "PicoCalc",
    .gamepad_type = kGamepadTypeNone,
    .graphics_type = kGraphicsTypePicomiteLcd,
    .play_modfile_params = kPlayModfileTypeWithInterrupt,
    .has_cmd_flash = true,
    .has_cmd_framebuffer = true,
    .has_mminfo_cpuspeed = true,
    .has_mminfo_drive = true,
    .has_mminfo_pin = true,
    .hres = 320,
    .vres = 320,
    .foreground = RGB_GREEN,
    .background = RGB_BLACK,
    .console = kBoth
};

static Features features_picomite_hdmi = {
    .name = "PicoMiteHDMI",
    .simple_name = "PicoMiteHDMI",
    .device = "PicoMiteHDMI",
    .platform = "",
    .gamepad_type = kGamepadTypePicomiteSnes,
    .graphics_type = kGraphicsTypePicomiteHdmi,
    .play_modfile_params = kPlayModfileTypeWithInterrupt,
    .has_cmd_flash = true,
    .has_cmd_framebuffer = true,
    .has_cmd_mode = true,
    .has_mminfo_cpuspeed = true,
    .has_mminfo_drive = true,
    .has_mminfo_ps2 = true,
    .has_mminfo_pin = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kBoth
};

static Features features_picomite_vga = {
    .name = "PicoMiteVGA",
    .simple_name = "PicoMiteVGA",
    .device = "PicoMiteVGA",
    .platform = "",
    .gamepad_type = kGamepadTypePicomiteSnes,
    .graphics_type = kGraphicsTypePicomiteVga,
    .play_modfile_params = kPlayModfileTypeWithInterrupt,
    .has_cmd_flash = true,
    .has_cmd_framebuffer = true,
    .has_cmd_mode = true,
    .has_mminfo_cpuspeed = true,
    .has_mminfo_drive = true,
    .has_mminfo_ps2 = true,
    .has_mminfo_pin = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kBoth
};

static Features features_picomite_vga_usb = {
    .name = "PicoMiteVGAUSB",
    .simple_name = "PicoMiteVGAUSB",
    .device = "PicoMiteVGAUSB",
    .platform = "",
    .gamepad_type = kGamepadTypePicomiteUsb,
    .graphics_type = kGraphicsTypePicomiteVga,
    .play_modfile_params = kPlayModfileTypeWithInterrupt,
    .has_cmd_flash = true,
    .has_cmd_framebuffer = true,
    .has_cmd_mode = true,
    .has_mminfo_cpuspeed = true,
    .has_mminfo_drive = true,
    .has_mminfo_pin = true,
    .has_mminfo_usb = true,
    .has_fun_keydown = true,
    .foreground = RGB_WHITE,
    .background = RGB_BLACK,
    .console = kBoth
};

MmResult features_init(Features *features, OptionsSimulate simulate) {
    LOG_FN_ENTRY("features=%p, simulate=%d", features, simulate);

    Features *new_features = NULL;

    switch (simulate) {
        case kSimulateCmm2:
            new_features = &features_cmm2;
            break;
        case kSimulateMmb4l:
            new_features = &features_mmb4l;
            break;
        case kSimulateMmb4w:
            new_features = &features_mmb4w;
            break;
        case kSimulateGamemite:
            new_features = &features_gamemite;
            break;
        case kSimulatePicocalc:
            new_features = &features_picocalc;
            break;
        case kSimulatePicomiteHdmi:
            new_features = &features_picomite_hdmi;
            break;
        case kSimulatePicomiteVga:
            new_features = &features_picomite_vga;
            break;
        case kSimulatePicomiteVgaUsb:
            new_features = &features_picomite_vga_usb;
            break;
        default:
            LOG_FN_EXIT("result=%d", kInternalFault);
            return kInternalFault;
    }

    memcpy(features, new_features, sizeof(Features));
    mmb_options.console = features->console;

    LOG_FN_EXIT("result=%d", kOk);
    return kOk;
}
