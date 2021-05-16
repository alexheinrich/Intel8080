#include <SDL2/SDL.h>
#include <stdbool.h>

#include "emulator8080.h"

void stop_loop(uint8_t c);
void play_sound(uint8_t c);
void loop_sound(uint8_t c);

void sdl_init();
bool sdl_exec(state8080 *state);
void sdl_quit();
