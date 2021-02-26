#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char registers[8] = {'b', 'c', 'd', 'e', 'h', 'l', 'm', 'a'};

typedef struct condition_flags {
    uint8_t s; // sign
    uint8_t z; // zero
    uint8_t ac;// auxiliary carry over
    uint8_t p; // is_even_parity
    uint8_t c; // carry
} condition_flags;

typedef struct state8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    size_t pc;
    uint16_t sp;
    uint8_t *memory;
    struct condition_flags cf;
    uint8_t interrupts_enabled;
} state8080;

void unimplemented_instruction(state8080 *state)
{
    (void) state;
    printf("Unimplemented Instruction: %02x\n", state->memory[state->pc]);
    //exit(1);
}

uint8_t is_even_parity(uint8_t number)
{
  uint8_t is_even = 1;
  for (int8_t i = 0; i < 8; i++) {
    if (number & 0x01) {
      is_even = !is_even;
    }

    number = number >> 1;
  }

  return is_even;
}

uint8_t *lookup_register(uint8_t register_number, state8080 *state)
{
    switch (register_number) {
        case 0:
            return &(state->b);
            break;
        case 1:
            return &(state->c);
            break;
        case 2:
            return &(state->d);
            break;
        case 3:
            return &(state->e);
            break;
        case 4:
            return &(state->h);
            break;
        case 5:
            return &(state->l);
            break;
        case 6:
            // access memory at (HL)
            break;
        case 7:
            return &(state->a);
            break;
        default:
            printf("Undefined Register");
            exit(1);
            break;
    }

    // case 6
    //uint16_t address = (((uint16_t) state->h) << 8) + (uint16_t) state->l;
    uint16_t address = (uint16_t) (state->h << 8) + (uint16_t) state->l;
    uint16_t offset = address / sizeof(uint8_t);
    //printf("%u\n", offset);
    return &(state->memory[offset]);
}

void emulate8080(state8080 *state)
{
    unsigned char opcode = state->memory[state->pc];

    uint16_t buffer;
    
    // add (a <- (a + source))
    if ((opcode & 0xf8) == 0x80) {
        printf("%x02\n", opcode);
        uint8_t source = opcode & 0x07;
        uint8_t *source_address = lookup_register(source, state);

        buffer = (uint16_t) state->a + (uint16_t) *source_address;

        // is this the same as:
        // (buffer & 0xff) == 0
        if (buffer == 0) {
            state->cf.z = 1;
        } else {
            state->cf.z = 0;
        }

        if (buffer & 0x80) {
            state->cf.s = 1;
        } else {
            state->cf.s = 0;
        }

        if (buffer > 0xff) {
            state->cf.c = 1;
        } else {
            state->cf.c = 0;
        }

        if (is_even_parity((uint8_t) buffer)) {
            state->cf.p = 1;
        } else {
            state->cf.p = 0;
        }

        state->a = buffer & 0xFF;
        printf("add\n");
    }

    switch (opcode) {
        case 0x00:          // nop
            break;
        case 0x80:          // add b
            break;

        default:
            unimplemented_instruction(state);
            break;
    };

    state->pc++;
}

int32_t main(int argc, char **argv)
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
        exit(1);
    }

    if (fread(buffer, sizeof(uint8_t), fsize, f) != fsize) {
        printf("fread() failed. Errno: %s.\n", strerror(errno));
        exit(1);
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
            .c = 0
        },
        .interrupts_enabled = 0
    };

    while (state.pc < fsize) {
        emulate8080(&state);
    }

    if (fclose(f) != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    free(buffer);

    return 0;
}
