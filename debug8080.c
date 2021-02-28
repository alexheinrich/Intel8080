#include "emulator8080.h"
#include "disassembler8080.h"

#include <stdbool.h>
#include <stdio.h>

const bool show_flags = false;

void print_registers8080(state8080 *state)
{
    printf("--------------------\n");
    printf("Registers:\n");
    printf("a: %u\n", state->a);
    printf("b: %u\n", state->b);
    printf("c: %u\n", state->c);
    printf("d: %u\n", state->d);
    printf("e: %u\n", state->e);
    printf("h: %u\n", state->h);
    printf("l: %u\n", state->l);
}

void print_condition_flags8080(state8080 *state)
{
    printf("--------------------\n");
    printf("Condition Flags:\n");
    printf("Sign:   %d\n", state->cf.s);
    printf("Zero:   %d\n", state->cf.z);
    printf("Aux CY: %d\n", state->cf.ac);
    printf("Parity: %d\n", state->cf.p);
    printf("CY:     %d\n", state->cf.c);
}

void print_state8080(state8080 *state, bool after_op)
{
    if (after_op == false) {
        printf("====================\n");
        printf("Operation:\n");
        disassemble_op8080(state->memory, state->pc);
    }

    print_registers8080(state);

    if (show_flags) {
        print_condition_flags8080(state);
    }

    if (after_op == true) {
        printf("====================\n\n");
    }
}
