#pragma once

#include "emulator8080.h"

#include <stdint.h>
#include <stdio.h>

void exit_and_free(uint8_t *buffer);
FILE *open_f(char *filepath);
size_t get_fsize(FILE *f);
state8080 load_rom(FILE *f, size_t fsize);

