#include "utils8080.h"
#include "emulator8080.h"
#include "disassembler8080.h"
#include "test8080.h"
#include "video_driver.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Usage: emulator8080 [-dt] [file]\n");
    printf("  Emulate an Intel8080 system for a rom file.\n");
    printf("  The following additional options are available:\n");
    printf("  -d\tdisassemble entered rom file\n");
    printf("  -t\trun tests on the emulator (no file argument used)\n");
}

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "-t") == 0) {
        FILE *f = open_f("test/test_cases.txt");
        int n = 0;
        while (exec_test_case(f)) {
            printf("------ %d\n", n);
            n++;
        }

        close_f(f);
    } else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 3) {
            printf("Enter file to disassemble\n");
            return 1;
        }

        state8080 state;
        ssize_t fsize = load_rom(&state, argv[2]);
        if (fsize < 0) {
            return 1;
        }
        
        size_t bc = 0;

        while (bc < (size_t) fsize) {
            bc += disassemble_op8080(state.memory, bc);
        }

        unload_rom(&state);
    } else {
        state8080 state;
        if (load_rom(&state, argv[1]) < 0) {
            return 1;
        }
        
        int n = 0;
        printf("cycle: %d\n", n);
        while (emulate8080(&state, true)) {
            n++;
            printf("cycle: %d\n", n);
            if (n > 100000) break;
        }

        unload_rom(&state);
    }

    return 0;
}

