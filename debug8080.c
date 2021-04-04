#include "emulator8080.h"
#include "disassembler8080.h"

#include <stdbool.h>
#include <stdio.h>

const bool show_flags = true;

void print_registers8080(const state8080 *state)
{
    printf("--------------------\n");
    printf("Registers:\n");
    printf("a: 0x%02x\n", state->a);
    printf("b: 0x%02x\n", state->b);
    printf("c: 0x%02x\n", state->c);
    printf("d: 0x%02x\n", state->d);
    printf("e: 0x%02x\n", state->e);
    printf("h: 0x%02x\n", state->h);
    printf("l: 0x%02x\n", state->l);
    printf("sp: 0x%04x\n", state->sp);
}

void print_condition_flags8080(const state8080 *state)
{
    printf("--------------------\n");
    printf("Condition Flags:\n");
    printf("Sign:   %d\n", state->cf.s);
    printf("Zero:   %d\n", state->cf.z);
    printf("Aux CY: %d\n", state->cf.ac);
    printf("Parity: %d\n", state->cf.p);
    printf("CY:     %d\n", state->cf.cy);
}

void print_state_pre(const state8080 *state)
{
    printf("====================\n");
    printf("Operation:\n");
    printf("PC: %hu %04x\n", state->pc, state->pc);
    disassemble_op8080(state->memory, (size_t) state->pc);

    print_registers8080(state);

    if (show_flags) {
        print_condition_flags8080(state);
    }
}

void print_state_post(const state8080 *state)
{
    print_registers8080(state);

    if (show_flags) {
        print_condition_flags8080(state);
    }

    printf("====================\n\n");
}

