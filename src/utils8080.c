#include "emulator8080.h"
#include "utils8080.h"

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void exit_and_free(uint8_t *buffer)
{
    free(buffer);
    exit(1);
}

FILE *open_f(const char *filepath)
{
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("failed to open: %s. Errno: %s.\n", filepath, strerror(errno));
        exit(1);
    }
    
    return f;
}

bool close_f(FILE *f)
{
    if (fclose(f) != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        return false;
    }

    return true;
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

ssize_t load_rom(state8080 *state, const char *filepath)
{
    FILE *f = open_f(filepath);
    size_t fsize = get_fsize(f);

    // 64KiB
    uint8_t *buffer = malloc(0x10000);
    if (buffer == NULL) {
        printf("malloc() failed. Errno: %s.\n", strerror(errno));
        close_f(f);

        return -1;
    }

    memset(buffer, 0, 0x10000);

    if (fread(buffer, sizeof(uint8_t), fsize, f) != fsize) {
        printf("fread() failed. Errno: %s.\n", strerror(errno));
        free(buffer);
        close_f(f);

        return -1;
    }

    close_f(f);

    *state = (state8080) {
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
            .pad5 = 0, // always 0
            .ac = 0,
            .pad3 = 0, // always 0
            .p = 0,
            .pad1 = 1, // always 1
            .cy = 0
        },
        .interrupts_enabled = 0
    };
    
    return (ssize_t) fsize;
}

void unload_rom(state8080 *state)
{
    if (state->memory != NULL) {
        free(state->memory);
    }
}

void load_hiscore(state8080 *state)
{
    FILE *dat = open_f("hiscore.dat");

    int hiscore;
    if (fscanf(dat, "%04x", &hiscore) < 0) {
        fprintf(stderr, "fscanf failed to read hiscore.\n");
        return;
    }
    
    uint8_t hs_lo = (uint8_t) hiscore;
    uint8_t hs_hi = (uint8_t) (hiscore >> 8);

    state->memory[0x1bf4] = hs_lo;
    state->memory[0x1bf5] = hs_hi;

    if (!close_f(dat)) {
        fprintf(stderr, "Unable to close hi score file.\n");
    }
}

void save_hiscore(state8080 *state)
{
    FILE *dat = fopen("hiscore.dat", "wb");
    uint16_t hiscore = (uint16_t) ((state->memory[0x20f5] << 8) | (state->memory[0x20f4]));
    fprintf(dat, "%04x", hiscore);

    if (!close_f(dat)) {
        fprintf(stderr, "Unable to close hi score file.\n");
    }
}
