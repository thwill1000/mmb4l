/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "sdl2_stubs.h"

const char *window = "window";
const char *renderer = "renderer";
const char *texture = "texture";

SDL_GameController* (*mock_SDL_GameControllerOpen)(int joystick_index) = NULL;
SDL_JoystickID (*mock_SDL_JoystickInstanceID)(SDL_Joystick *joystick) = NULL;
SDL_Joystick* (*mock_SDL_JoystickOpen)(int device_index) = NULL;

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
     // Need to return something if successful.
     // The real SDL_Window is an opaque type.
     return (SDL_Window *) window;
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags) {
     // Need to return something if successful.
     // The real SDL_Renderer is an opaque type.
    return (SDL_Renderer *) renderer;
}

SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h) {
     // Need to return something if successful.
     // The real SDL_Texture is an opaque type.
    return (SDL_Texture *) texture;
}

void SDL_DestroyRenderer(SDL_Renderer *renderer) {
}

void SDL_DestroyTexture(SDL_Texture *texture) {
}

void SDL_DestroyWindow(SDL_Window *window) {
}

void SDL_GameControllerClose(SDL_GameController *gamecontroller) {
}

char *SDL_GameControllerMapping(SDL_GameController *gamecontroller) {
    return NULL;
}

SDL_GameController* SDL_GameControllerOpen(int joystick_index) {
    return mock_SDL_GameControllerOpen
            ? mock_SDL_GameControllerOpen(joystick_index)
            : NULL;
}

int SDL_GameControllerEventState(int state) {
    return 0;
}

SDL_bool SDL_GameControllerHasRumble(SDL_GameController *gamecontroller) {
    return SDL_FALSE;
}

int SDL_GameControllerRumble(SDL_GameController *gamecontroller, Uint16 low_frequency_rumble,
                             Uint16 high_frequency_rumble, Uint32 duration_ms) {
    return 0;
}

int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode * mode) {
    mode->h = 1080;
    mode->w = 1920;
    return 0;
}

const char* SDL_GetError(void) {
    return "Stubbed error message";
}

Uint32 SDL_GetTicks(void) {
    return 0;
}

Uint32 SDL_GetWindowID(SDL_Window *window) {
    return 0;
}

int SDL_Init(Uint32 flags) {
    return 0;
}

void SDL_JoystickClose(SDL_Joystick *joystick) {
}

SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick *joystick) {
    return mock_SDL_JoystickInstanceID
            ? mock_SDL_JoystickInstanceID(joystick)
            : 0;
}

SDL_Joystick* SDL_JoystickOpen(int device_index) {
    return mock_SDL_JoystickOpen
            ? mock_SDL_JoystickOpen(device_index)
            : NULL;
}

int SDL_NumJoysticks(void) {
    return 0;
}

int SDL_PollEvent(SDL_Event *event) {
    return 0;
}

int SDL_RenderClear(SDL_Renderer* renderer) {
    return 0;
}

int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                   const SDL_Rect *dstrect) {
    return 0;
}

void SDL_RenderPresent(SDL_Renderer *renderer) {
}

int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return 0;
}

int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch) {
    return 0;
}
