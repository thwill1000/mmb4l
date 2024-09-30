/*-*****************************************************************************

MMBasic for Linux (MMB4L)

events.c

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

#include "console.h"
#include "error.h"
#include "events.h"
#include "gamepad.h"
#include "graphics.h"
#include "interrupt.h"
#include "keyboard.h"
#include "utility.h"

// Defined in "core/MMBasic.c"
extern const char *CurrentLinePtr;

static const char* NO_ERROR = "";
static bool events_initialised = false;

MmResult events_init() {
    if (events_initialised) return kOk;
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO) < 0) {
        return kEventsApiError;
    }
    events_initialised = true;
    return kOk;
}

const char* events_last_error() {
    const char* emsg = SDL_GetError();
    return emsg && *emsg ? emsg : NO_ERROR;
}

void events_pump() {
    if (!events_initialised) return;

    MmResult result = kOk;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERAXISMOTION:
                // printf("Controller axis: device idx: %d, axis: %d, value: %d\n", event.caxis.which, event.caxis.axis, event.caxis.value);
                result = gamepad_on_analog(event.caxis.which, event.caxis.axis, event.caxis.value);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                result = gamepad_on_button_down(event.cbutton.which, event.cbutton.button);
                break;
            case SDL_CONTROLLERBUTTONUP:
                result = gamepad_on_button_up(event.cbutton.which, event.cbutton.button);
                break;
            case SDL_CONTROLLERDEVICEADDED:
                // printf("Controller added, device idx: %d\n", event.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                // printf("Controller removed, instance id: %d\n", event.cdevice.which);
                break;
            case SDL_KEYDOWN:
                result = keyboard_key_down(&event.key.keysym);
                break;

            case SDL_KEYUP:
                result = keyboard_key_up(&event.key.keysym);
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE: {
                        MmSurfaceId windowId = graphics_find_window(event.window.windowID);
                        if (windowId == -1) ERROR_ON_FAILURE(kInternalFault);
                        if (CurrentLinePtr) {
                            interrupt_fire_window_close(windowId);
                        } else {
                            ERROR_ON_FAILURE(
                                graphics_surface_destroy(&graphics_surfaces[windowId]));
                        }
                        break;
                    }

                    default:
                        break;
                }
                break;

            default:
                break;
        }
        if (FAILED(result)) error_throw(result);
    }
}
