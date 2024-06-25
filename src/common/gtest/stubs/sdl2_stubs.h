/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(SDL_STUBS_H)
#define SDL_STUBS_H

#include <SDL.h>

typedef struct _SDL_Joystick {
    int id;
} SDL_Joystick;

typedef struct _SDL_GameController {
    int id;
} SDL_GameController;

extern SDL_GameController* (*mock_SDL_GameControllerOpen)(int joystick_index);
extern SDL_JoystickID (*mock_SDL_JoystickInstanceID)(SDL_Joystick *joystick);
extern SDL_Joystick* (*mock_SDL_JoystickOpen)(int device_index);

#endif // #if !defined(SDL_STUBS_H)
