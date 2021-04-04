#include "emulator8080.h"
#include "utils.h"

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void exit_and_free(uint8_t *buffer)
{
    free(buffer);
    exit(1);
}

FILE *open_f(char *filepath)
{
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("fopen() failed to open: %s. Errno: %s.\n", filepath, strerror(errno));
        exit(1);
    }
    
    return f;
}

size_t get_fsize(FILE *f)
{
    if (fseek(f, 0L, SEEK_END) < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    off_t fsize_off = ftell(f);
    if (fsize_off < 0) {
        printf("ftell() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }
    size_t fsize = (size_t) fsize_off;

    if (fseek(f, 0L, SEEK_SET) < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    return fsize;
}

state8080 load_rom(FILE *f, size_t fsize)
{
    // 64KiB
    uint8_t *buffer = malloc(0x10000);
    if (buffer == NULL) {
        printf("malloc() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }
    memset(buffer, 0, 0x10000);

    if (fread(buffer, sizeof(uint8_t), fsize, f) != fsize) {
        printf("fread() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(buffer);
    }

    state8080 state = {
        .a = 0,
        .b = 0,
        .c = 0,
        .d = 0,
        .e = 0,
        .h = 0,
        .l = 0,
        .pc = 0,
        .sp = 0,
        .memory = buffer,
        .cf = {
            .s = 0,
            .z = 0,
            .ac = 0,
            .p = 0,
            .cy = 0
        },
        .interrupts_enabled = 0
    };

    return state;
}
