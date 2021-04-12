#include "emulator8080.h"
#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void get_instr_opbytes(char *str_ptr, uint8_t *opbytes)
{
    printf("%s", str_ptr);
    for (uint32_t c = 0; c < 3; ++c) {
        char *token;
        token = strsep(&str_ptr, "\n\t ");
        opbytes[c] = (uint8_t) strtol(token, NULL, 16);
    }
}

static void init_state_mem(state8080 *state, const uint8_t *opbytes)
{
    load_rom(state, "test/ram.dat");
    
    state->pc = 0x000d;
    // TODO exec first 5 instructions
    memcpy(state->memory + state->pc, opbytes, 3);
}

static bool compare_states(const state8080 *source, const state8080 *target)
{
    bool passed = true;

    if (source->a != target->a) {
        printf("Register a not equal! Source: %02x Target: %02x\n", source->a, target->a);
        passed = false;
    }
    
    if (source->b != target->b) {
        printf("Register b not equal! Source: %02x Target: %02x\n", source->b, target->b);
        passed = false;    
    }
    
    if (source->c != target->c) {
        printf("Register c not equal! Source: %02x Target: %02x\n", source->c, target->c);
        passed = false;    
    }
    
    if (source->d != target->d) {
        printf("Register d not equal! Source: %02x Target: %02x\n", source->d, target->d);
        passed = false;    
    }

    if (source->e != target->e) {
        printf("Register e not equal! Source: %02x Target: %02x\n", source->e, target->e);
        passed = false;    
    }
    
    if (source->h != target->h) {
        printf("Register h not equal! Source: %02x Target: %02x\n", source->h, target->h);
        passed = false;    
    }

    if (source->l != target->l) {
        printf("Register l not equal! Source: %02x Target: %02x\n", source->l, target->l);
        passed = false;    
    }

    if (source->cf.cy != target->cf.cy) {
        printf("Flag cy not equal! Source: %u Target: %u\n", source->cf.cy, target->cf.cy);
        passed = false;    
    }

    if (source->cf.p != target->cf.p) {
        printf("Flag p not equal! Source: %u Target: %u\n", source->cf.p, target->cf.p);
        passed = false;    
    }

    if (source->cf.ac != target->cf.ac) {
        printf("Flag ac not equal! Source: %u Target: %u\n", source->cf.ac, target->cf.ac);
        passed = false;    
    }

    if (source->cf.z != target->cf.z) {
        printf("Flag z not equal! Source: %u Target: %u\n", source->cf.z, target->cf.z);
        passed = false;    
    }

    if (source->cf.s != target->cf.s) {
        printf("Flag s not equal! Source: %u Target: %u\n", source->cf.s, target->cf.s);
        passed = false;    
    }

    if (!passed) {
        printf(KRED "TEST FAILED.\n" KNRM);
    }

    return passed;
}

static void parse_state(state8080 *state, char *str_ptr)
{
    char *token;
    while((token = strsep(&str_ptr, "\n\t ")) != NULL) {
        char *reg = strsep(&token, ":"); 

        if (strcmp(reg, "a") == 0) {
            state->a = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "b") == 0) {
            state->b = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "c") == 0) {
            state->c = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "d") == 0) {
            state->d = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "e") == 0) {
            state->e = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "h") == 0) {
            state->h = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "l") == 0) {
            state->l = (uint8_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "sp") == 0) {
            state->sp = (uint16_t) strtol(token, NULL, 16);
        } else if (strcmp(reg, "sf") == 0) {
            state->cf.s = atoi(token) != 0;
        } else if (strcmp(reg, "zf") == 0) {
            state->cf.z = atoi(token) != 0;
        } else if (strcmp(reg, "af") == 0) {
            state->cf.ac = atoi(token) != 0;
        } else if (strcmp(reg, "pf") == 0) {
            state->cf.p = atoi(token) != 0;
        } else if (strcmp(reg, "cf") == 0) {
            state->cf.cy = atoi(token) != 0;
        }
    }
}

static void parse_mem_write(char *str_ptr, state8080 *state)
{
    char *token;
    while((token = strsep(&str_ptr, "\n\t ")) != NULL) {
        if (*token != '\0') {
            char *mem_loc = strsep(&token, ":");
            uint16_t mem_loc_hex = (uint16_t) (strtol(mem_loc, NULL, 16));
            uint8_t target = (uint8_t) (strtol(token, NULL, 16));
            if (state->memory[mem_loc_hex] != target) {
                printf(KRED "TEST FAILED.\n" KNRM);
                printf("Memory write incorrect:");
                printf("state->memory[%04x]: %02x ", mem_loc_hex, state->memory[mem_loc_hex]);
                printf("target: %02x\n", target);
            }
        }
    }
}

static void parse_io_write(char *str_ptr, state8080 *state)
{
    (void) str_ptr;
    (void) state;
    // TODO
}

bool exec_test_case(FILE *f)
{
    char *line_ptr = NULL;
    size_t n = 0; 
    bool success = true;

    uint8_t opbytes[3];
    state8080 state;
    for (uint32_t i = 0; i < 5; ++i) {
        if (getline(&line_ptr, &n, f) == -1) {
            success = false;
            break;
        }

        char *line_cpy = line_ptr;
        printf("%s", line_ptr); 

        char *h_str = strsep(&line_cpy, "\n\t ");

        if (strcmp(h_str, "inst") == 0) {
            get_instr_opbytes(line_cpy, opbytes);
        } else if (strcmp(h_str, "pre") == 0) {
            init_state_mem(&state, opbytes);
            parse_state(&state, line_cpy);
            emulate8080(&state);
        } else if (strcmp(h_str, "post") == 0) {
            state8080 target_state;
            parse_state(&target_state, line_cpy);
            if (!compare_states(&state, &target_state)) {
                success = false;
                break;
            }
        } else if (strcmp(h_str, "ram") == 0) {
            parse_mem_write(line_cpy, &state);
        } else if (strcmp(h_str, "io") == 0) {
            parse_io_write(line_cpy, &state);
        }
    }

    free(state.memory);
    if (line_ptr != NULL) {
        printf("line_ptr %p\n", line_ptr);
        free(line_ptr);
    }

    return success;
}
