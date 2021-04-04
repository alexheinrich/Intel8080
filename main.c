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
        FILE *f = open_f("test/test_cases.txt");
        char *line_ptr = NULL;
        size_t n = 0; 
        
        int count = 500;

        while (count > 0) {
            printf("------ %d\n", count);
            exec_test_case(&line_ptr, &n, f);
            count--;
        }

        return 0;
    }

    FILE *f = open_f(argv[1]);
    size_t fsize = get_fsize(f);
    state8080 state = load_rom(f, fsize);
    
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        size_t bc = 0;

        // Disassemble 
        while (bc < fsize) {
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

    if (fclose(f) != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(state.memory);
    }

    free(state.memory);

    return 0;
}

