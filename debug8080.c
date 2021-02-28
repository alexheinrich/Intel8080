#include "emulator8080.h"

#include <stdio.h>

void print_registers8080(state8080 *state)
{
    printf("--------------------\n");
    printf("Registers:\n");
    printf("--------------------\n");
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
    printf("--------------------\n");
    printf("Sign:   %d\n", state->cf.s);
    printf("Zero:   %d\n", state->cf.z);
    printf("Aux CY: %d\n", state->cf.ac);
    printf("Parity: %d\n", state->cf.p);
    printf("CY:     %d\n", state->cf.c);
}

void print_state8080(state8080 *state)
{
    printf("====================\n");
    printf("Debug:\n");
    print_registers8080(state);
    print_condition_flags8080(state);
    printf("====================\n");
}
