#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool s; // sign
    bool z; // zero
    bool ac;// auxiliary carry over
    bool p; // parity
    bool cy; // carry over
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

