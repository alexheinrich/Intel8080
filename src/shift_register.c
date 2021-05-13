#include "emulator8080.h"

#include <stdint.h>

static uint16_t reg = 0x0000;
static uint8_t shift_offset = 0;

void sreg_push_val(uint8_t val)
{
    reg = (uint16_t) ((val << 8) | (reg >> 8));
}

void sreg_set_shift(uint8_t set)
{
    shift_offset = set & 0x07;
}

uint8_t sreg_get_val(void)
{
    return (uint8_t) (reg >> (8 - shift_offset));
}

