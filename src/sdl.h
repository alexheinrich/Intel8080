#include <SDL2/SDL.h>
#include <stdbool.h>

#include "emulator8080.h"

void sdl_init();
bool sdl_exec(state8080 *state);
void sdl_quit();
