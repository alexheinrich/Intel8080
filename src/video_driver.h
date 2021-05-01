#include <SDL2/SDL.h>
#include <stdbool.h>

#include "emulator8080.h"

void video_init();
bool video_exec(state8080 *state);
void video_quit();
