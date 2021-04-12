#pragma once

#include "emulator8080.h"

#include <stdint.h>
#include <stdio.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void exit_and_free(uint8_t *buffer);
FILE *open_f(const char *filepath);
bool close_f(FILE *f);
size_t get_fsize(FILE *f);
ssize_t load_rom(state8080 *state, const char *filepath);

