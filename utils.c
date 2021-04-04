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

