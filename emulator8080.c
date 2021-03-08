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
                printf("Mem: 0x%02x\n", state->memory[address]);
                return &state->memory[address];
            }
        case 7:
            return &state->a;
        default:
            printf("Error: Undefined Register.\n");
            exit(1);
    }

}

uint16_t address_from_register_pair(uint8_t first_register, uint8_t second_register)
{
    return (uint16_t) ((first_register << 8) + second_register);
}

bool emulate8080(state8080 *state)
{
    unsigned char opcode = state->memory[state->pc];
    uint8_t opbytes = 1;

    uint16_t buffer;
    print_state_pre(state);

    // mov (and hlt)
    if ((opcode & 0xc0) == 0x40) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);
        uint8_t destination = (opcode >> 3) & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);

        // hlt
        if (source == 0x06 && destination == 0x06) {
            return false;
        } else {
            *destination_pointer = source_value;
        }
    }


    // add (a <- a + source)
    if ((opcode & 0xf8) == 0x80) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = (uint16_t) (state->a + source_value);

        state->a = (uint8_t) buffer;
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = buffer > 0xff;
        state->cf.p = is_even_parity(state->a);
    }

    // adc (a <- a + source + cy)
    if ((opcode & 0xf8) == 0x88) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = (uint16_t) (state->a + source_value + state->cf.cy);

        state->a = (uint8_t) buffer;
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = buffer > 0xff;
        state->cf.p = is_even_parity(state->a);
    }

    // sub (a <- a - source)
    if ((opcode & 0xf8) == 0x90) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~source_value + 0x01;

        state->a = (uint8_t) buffer;
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = !(buffer > 0xff);
        state->cf.p = is_even_parity(state->a);
    }
    
    // sbb (a <- a - source)
    if ((opcode & 0xf8) == 0x98) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~(source_value + state->cf.cy) + 0x01;

        state->a = (uint8_t) buffer;
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = !(buffer > 0xff);
        state->cf.p = is_even_parity(state->a);
    }

    // ana (a <- a & source)
    if ((opcode & 0xf8) == 0xa0) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a & source_value);
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = false;
        state->cf.p = is_even_parity(state->a);
    }

    // xra (a <- a ^ source)
    if ((opcode & 0xf8) == 0xa8) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a ^ source_value);
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = false;
        state->cf.p = is_even_parity(state->a);
    }

    // ora (a <- a | source)
    if ((opcode & 0xf8) == 0xb0) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        state->a = (uint8_t) (state->a | source_value);
        state->cf.z = state->a == 0x00;
        state->cf.s = (state->a & 0x80) == 0x80;
        state->cf.cy = false;
        state->cf.p = is_even_parity(state->a);
    }

    // cmp (a < source)
    if ((opcode & 0xf8) == 0xb8) {
        uint8_t source = opcode & 0x07;
        uint8_t source_value = *lookup_register(source, state);

        buffer = state->a + (uint8_t) ~source_value + 0x01;

        uint8_t result = (uint8_t) buffer;
        state->cf.z = result == 0x00;
        state->cf.s = (result & 0x80) == 0x80; 
        state->cf.cy = !(buffer > 0xff);
        state->cf.p = is_even_parity(result);
    }

    // inr
    if ((opcode & 0xc7) == 0x04) {
        uint8_t destination = opcode >> 3 & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);

        buffer = *destination_pointer + 1;
        *destination_pointer = (uint8_t) buffer;
        state->cf.z = *destination_pointer == 0x00;
        state->cf.s = (*destination_pointer & 0x80) == 0x80;
        state->cf.p = is_even_parity(*destination_pointer);
    }
    
    // dcr
    if ((opcode & 0xc7) == 0x05) {
        uint8_t destination = opcode >> 3 & 0x07;
        uint8_t *destination_pointer = lookup_register(destination, state);

        buffer = *destination_pointer - 1;
        *destination_pointer = (uint8_t) buffer;
        state->cf.z = *destination_pointer == 0x00;
        state->cf.s = (*destination_pointer & 0x80) == 0x80;
        state->cf.p = is_even_parity(*destination_pointer);
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
                uint16_t address = address_from_register_pair(state->b, state->c);
                state->memory[address] = state->a;
            }
        case 0x03: // inx b
            state->b = (uint8_t) (state->b + 1);
            state->c = (uint8_t) (state->c + 1);
            break;

        // case 0x04: inr b
        // case 0x05: dcr b
        case 0x06: // mvi b, d8

        case 0x11: // lxi d
            state->d = state->memory[state->pc + 2];
            state->e = state->memory[state->pc + 1];
            opbytes = 3;
            break;

        case 0x21: // lxi h
            state->h = state->memory[state->pc + 2];
            state->l = state->memory[state->pc + 1];
            opbytes = 3;
            break;


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

    uint8_t *buffer = malloc(fsize);
    if (buffer == NULL) {
        printf("malloc() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(buffer);
    }

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


    // disassemble
    if (fclose(f) != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        exit_and_free(buffer);
    }

    free(buffer);

    return 0;
}
