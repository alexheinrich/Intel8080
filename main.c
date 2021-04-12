#include "utils.h"
#include "emulator8080.h"
#include "disassembler8080.h"
#include "test8080.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "-t") == 0) {
        int count = 500;

        FILE *f = open_f("test/test_cases.txt");
        while (count > 0 && exec_test_case(f)) {
            printf("------ %d\n", count);
            count--;
        }

        close_f(f);

        return 0;
    }

    state8080 state;
    ssize_t fsize = load_rom(&state, argv[1]);
    if (fsize < 0) {
        return 1;
    }
    
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        size_t bc = 0;

        // Disassemble 
        while (bc < (size_t) fsize) {
            bc += disassemble_op8080(state.memory, bc);
        }
    } else {
        // Emulate
        bool emulate = true;
        int i = 0;
        while (emulate) {
            printf("cycle: %d\n", i);
            emulate = emulate8080(&state);
            i++;
        }
    }

    free(state.memory);

    return 0;
}

