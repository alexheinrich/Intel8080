#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t cy:1; // carry over
    uint8_t pad1:1;
    uint8_t p:1; // parity
    uint8_t pad3:1;
    uint8_t ac:1;// auxiliary carry over
    uint8_t pad5:1;
    uint8_t z:1; // zero
    uint8_t s:1; // sign
} condition_flags;

typedef struct {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t pc;
    uint16_t sp;
    uint8_t *memory;
    condition_flags cf;
    uint8_t interrupts_enabled;
} state8080;

void run_emulator(state8080 *state);
bool emulate_op8080(state8080 *state, bool debug);

