#include "emulator8080.h"
#include "debug8080.h"
#include "disassembler8080.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_even_parity(uint8_t number)
{
    uint32_t t1 = number ^ (number >> 4);
    uint32_t t2 = t1 ^ (t1 >> 2);
    uint32_t t3 = t2 ^ (t2 >> 1);

    return !(t3 & 0x01);
}

uint8_t *lookup_register(uint8_t register_number, state8080 *state)
{
    switch (register_number) {
        case 0:
            return &state->b;
        case 1:
            return &state->c;
        case 2:
            return &state->d;
        case 3:
            return &state->e;
        case 4:
            return &state->h;
        case 5:
            return &state->l;
        case 6: 
            {
                uint16_t address = (uint16_t) ((state->h << 8) + state->l);
                return &state->memory[address];
            }
        case 7:
            return &state->a;
        default:
            printf("Error: Undefined Register.\n");
            exit(1);
    }

}

uint32_t lookup_register_pair_source(uint8_t source_num, state8080 *state)
{
    uint8_t source_h, source_l;
    //assert(source_num < 0x04);

    switch(source_num) {
        case 0x00:
            source_h = state->b;
            source_l = state->c;
            break;
        case 0x01:
            source_h = state->d;
            source_l = state->e;
            break;
        case 0x02:
            source_h = state->h;
            source_l = state->l;
            break;
        case 0x03:
            source_h = (uint8_t) state->sp;
            source_l = (uint8_t) (state->sp >> 8);
            break;
        default:
            fprintf(stderr, "Unkown source: %02x lookup_register_pair\n", source_num);
            exit(1);
    }

    return (uint32_t) ((source_h << 8) + source_l);
}

void op_on_register_pair(uint8_t src_num, state8080 *state, uint16_t (*op)(uint16_t))
{
    switch(src_num) {
        case 0x00:
            state->b = (uint8_t) op(state->b);
            state->c = (uint8_t) op(state->c);
            break;
        case 0x01:
            state->d = (uint8_t) op(state->d);
            state->e = (uint8_t) op(state->e);
            break;
        case 0x02:
            state->h = (uint8_t) op(state->h);
            state->l = (uint8_t) op(state->l);
            break;
        case 0x03:
            state->sp = op(state->sp);
            break;
        default:
            fprintf(stderr, "Unkown source: %02x lookup_register_pair\n", src_num);
            exit(1);
    }
}

void set_szp(state8080 *state, uint8_t result)
{
    state->cf.z = result == 0x00;
    state->cf.s = (result & 0x80) == 0x80;
    state->cf.p = is_even_parity(result);
}

uint16_t dcr(uint16_t src)
{
    return (uint16_t) (src - 1);
}

uint16_t incr(uint16_t src)
{
    return src + 1;
}

bool emulate8080(state8080 *state)
{
    unsigned char opcode = state->memory[state->pc];
    uint8_t opbytes = 1;

    uint32_t buffer;
    print_state_pre(state);

    // mov (and hlt)
    if ((opcode & 0xc0) == 0x40) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);
        uint8_t destination = (opcode >> 3) & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);

        if (destination_pointer == NULL) {
            return false;
        }

        // hlt
        if (source == 0x06 && destination == 0x06) {
            return false;
        }

        *destination_pointer = source_value;
    }

    // mvi (destination <- source)
    if ((opcode & 0xc7) == 0x06) {
        uint8_t destination = (opcode >> 3) & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);

        if (destination_pointer == NULL) {
            return false;
        }

        *destination_pointer = state->memory[state->pc + 1];
        opbytes = 2;
    }

    // inr
    if ((opcode & 0xc7) == 0x04) {
        uint8_t destination = opcode >> 3 & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);
        
        if (destination_pointer == NULL) {
            return false;
        }

        buffer = *destination_pointer + 1;
        *destination_pointer = (uint8_t) buffer;
        set_szp(state, *destination_pointer);
    }
    
    // dcr
    if ((opcode & 0xc7) == 0x05) {
        uint8_t destination = opcode >> 3 & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);
        
        if (destination_pointer == NULL) {
            return false;
        }

        buffer = *destination_pointer - 1;
        *destination_pointer = (uint8_t) buffer;
        set_szp(state, *destination_pointer);
    }

    // add (a <- a + source)
    if ((opcode & 0xf8) == 0x80) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = (uint16_t) (state->a + source_value);

        state->a = (uint8_t) buffer;
        state->cf.cy = buffer > 0xff;

        set_szp(state, state->a);
    }

    // adc (a <- a + source + cy)
    if ((opcode & 0xf8) == 0x88) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = (uint16_t) (state->a + source_value + state->cf.cy);

        state->a = (uint8_t) buffer;
        state->cf.cy = buffer > 0xff;

        set_szp(state, state->a);
    }

    // sub (a <- a - source)
    if ((opcode & 0xf8) == 0x90) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~source_value + 0x01;

        state->a = (uint8_t) buffer;
        state->cf.cy = !(buffer > 0xff);

        set_szp(state, state->a);
    }
    
    // sbb (a <- a - source)
    if ((opcode & 0xf8) == 0x98) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~(source_value + state->cf.cy) + 0x01;

        state->a = (uint8_t) buffer;
        state->cf.cy = !(buffer > 0xff);

        set_szp(state, state->a);
    }

    // ana (a <- a & source)
    if ((opcode & 0xf8) == 0xa0) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a & source_value);
        state->cf.cy = false;

        set_szp(state, state->a);
    }

    // xra (a <- a ^ source)
    if ((opcode & 0xf8) == 0xa8) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a ^ source_value);
        state->cf.cy = false;

        set_szp(state, state->a);
    }

    // ora (a <- a | source)
    if ((opcode & 0xf8) == 0xb0) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a | source_value);
        state->cf.cy = false;

        set_szp(state, state->a);
    }

    // cmp (a < source)
    if ((opcode & 0xf8) == 0xb8) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~source_value + 0x01;

        uint8_t result = (uint8_t) buffer;
        state->cf.cy = !(buffer > 0xff);

        set_szp(state, result);
    }

    // dad
    if ((opcode & 0xcf) == 0x09) {
        uint8_t source_num = (uint8_t) ((opcode & 0x30) >> 4);
        uint32_t source = lookup_register_pair_source(source_num, state);

        buffer = (uint32_t) ((state->h << 8) + state->l) + source;
        state->h = (uint8_t) (buffer >> 8);
        state->l = (uint8_t) buffer;

        state->cf.cy = buffer > 0xffff;
    }
    
    // inx
    if ((opcode & 0xcf) == 0x03) {
        uint8_t reg_num = (uint8_t) ((opcode & 0x30) >> 4);
        op_on_register_pair(reg_num, state, incr);
    }

    // dcx
    if ((opcode & 0xcf) == 0x0b) {
        uint8_t reg_num = (uint8_t) ((opcode & 0x30) >> 4);
        op_on_register_pair(reg_num, state, dcr);
    }

    // sta/lda/shld/lhld
    if ((opcode & 0xe7) == 0x22) {
        uint8_t op_n = (opcode >> 3) & 0x03;
        uint8_t addr_h = state->memory[state->pc + 2];
        uint8_t addr_l = state->memory[state->pc + 1];
        uint16_t addr = (uint16_t)((addr_h << 8) + addr_l);

        switch (op_n) {
            case 0x00: // shld
                state->memory[addr] = state->h;
                state->memory[addr + 1] = state->l;
                break;
            case 0x01: // lhld
                state->h = state->memory[addr];
                state->l = state->memory[addr + 1];
                break;
            case 0x02: // sta
                state->memory[addr] = state->a;
                break;
            case 0x03: // lda
                state->a = state->memory[addr];
                break;
            default:
                fprintf(stderr, "Invalid op_n: %02x sta/lda/shld/lhld\n", op_n);
                exit(1);
        }

        opbytes = 3;
    }


    switch (opcode) {
        case 0x00: // nop
            break;
        case 0x01: // lxi b
            state->b = state->memory[state->pc + 2];
            state->c = state->memory[state->pc + 1];
            opbytes = 3;
            break;
        case 0x02: // stax b
            {
                uint16_t addr = (uint16_t) ((state->b << 8) + state->c);
                state->memory[addr] = state->a;
                printf("stax b: %02x\n", state->memory[addr]);
                break;
            }
        // case 0x03: inx b
        // case 0x04: inr b
        // case 0x05: dcr b
        // case 0x06: mvi b, d8
        case 0x07: // rlc
            buffer = state->a >> 7;
            state->a = (uint8_t) ((uint8_t) (state->a << 1) + buffer);
            state->cf.cy = buffer;
            break;
        // case 0x08: nop
        // case 0x09: dad b
        case 0x0a: // ldax b
            {
                uint16_t addr = (uint16_t) ((state->b << 8) + state->c);
                state->a = state->memory[addr];
                break;
            }
        // case 0x0b: dcx b
        // case 0x0c: inr c
        // case 0x0d: dcr c
        // case 0x0e: mvi c, d8
        case 0x0f: // rrc
            buffer = (uint8_t) (state->a << 7);
            state->a = (uint8_t) ((state->a >> 1) + buffer);
            state->cf.cy = buffer;
            break;
        // case 0x10: nop
        case 0x11: // lxi d
            state->d = state->memory[state->pc + 2];
            state->e = state->memory[state->pc + 1];
            opbytes = 3;
            break;
        case 0x12: // stax d
            {
                uint16_t addr = (uint16_t) ((state->d << 8) + state->e);
                state->memory[addr] = state->a;
                printf("stax d: %02x\n", state->memory[addr]);
                break;
            }
        // case 0x13: inx d
        // case 0x14: inr d
        // case 0x15: dcr d
        // case 0x16: mvi d, d8
        case 0x17: // ral
            buffer = state->a >> 7;
            state->a = (uint8_t) ((uint8_t) (state->a << 1) + state->cf.cy);
            state->cf.cy = buffer;
            break;
        // case 0x18: nop
        // case 0x19: dad d
        case 0x1a: // ldax d
            {
                uint16_t addr = (uint16_t) ((state->d << 8) + state->e);
                state->a = state->memory[addr];
                break;
            }
        // case 0x1b: dcx d
        // case 0x1c: inr e
        // case 0x1d: dcr e
        // case 0x1e: mvi e, d8
        case 0x1f: // rar
            buffer = (uint8_t) (state->a << 7);
            state->a = (uint8_t) ((state->a >> 1) + state->cf.cy);
            state->cf.cy = buffer;
            break;
        // case 0x20: nop
        case 0x21: // lxi h
            state->h = state->memory[state->pc + 2];
            state->l = state->memory[state->pc + 1];
            opbytes = 3;
            break;
        // case 0x22: shld adr
        // case 0x23: inx h
        // case 0x24: inr h
        // case 0x25: dcr h
        // case 0x26: mvi h, d8

        // case 0x28: nop
        // case 0x29: dad h
        // case 0x2a: lhld adr    
        // case 0x2b: dcx h
        // case 0x2c: inr l
        // case 0x2d: dcr l
        // case 0x2e: mvi l, d8

        // case 0x32: sta adr
        // case 0x33: inx sp
        // case 0x34: inr m
        // case 0x35: dcr m
        // case 0x36: mvi m, d8
        case 0x37: // stc
            state->cf.cy = 1;
            break;
        // case 0x38: nop
        // case 0x39: dad sp
        // case 0x3a: lda adr
        // case 0x3b: dcx sp
        // case 0x3c: inr a
        // case 0x3d: dcr a
        // case 0x3e: mvi a, d8
        case 0x3f: // cmc
            state->cf.cy = !state->cf.cy;
            break;
        // 0x40 - 0x7f mov
        // 0x80 - 0x87 add
        // 0x88 - 0x8f adc
        // 0x90 - 0x97 sub
        // 0x98 - 0x9f sbb
        // 0xa0 - 0xa7 ana
        // 0xa8 - 0xaf xra
        // 0xb0 - 0xb7 ora
        // 0xb8 - 0xbf cmp

        default:
            //unimplemented_instruction(state);
            break;
    }

    state->pc = state->pc + opbytes;

    print_state_post(state);
    return true;
}

void exit_and_free(uint8_t *buffer)
{
    free(buffer);
    exit(1);
}

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("fopen() failed to open: %s. Errno: %s.\n", argv[1], strerror(errno));
        exit(1);
    }

    if (fseek(f, 0L, SEEK_END) < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    off_t fsize_off = ftell(f);
    if (fsize_off < 0) {
        printf("ftell() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }
    size_t fsize = (size_t) fsize_off;

    if (fseek(f, 0L, SEEK_SET) < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    // 64KiB
    uint8_t *buffer = malloc(0x10000);
    if (buffer == NULL) {
        printf("malloc() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }
    memset(buffer, 0, 0x10000);

    if (fread(buffer, sizeof(uint8_t), fsize, f) != fsize) {
        printf("fread() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(buffer);
    }

    state8080 state = {
        .a = 0,
        .b = 0,
        .c = 0,
        .d = 0,
        .e = 0,
        .h = 0,
        .l = 0,
        .pc = 0,
        .sp = 0,
        .memory = buffer,
        .cf = {
            .s = 0,
            .z = 0,
            .ac = 0,
            .p = 0,
            .cy = 0
        },
        .interrupts_enabled = 0
    };
    
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        // Disassemble 
        while (state.pc < fsize) {
            state.pc += disassemble_op8080(state.memory, state.pc);
        }
    } else {
        // Emulate
        bool emulate = true;
        while (emulate) {
            emulate = emulate8080(&state);
        }
    }

    if (fclose(f) != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(buffer);
    }

    free(buffer);

    return 0;
}
