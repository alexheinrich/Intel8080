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

uint8_t *lkp_reg(uint8_t register_number, state8080 *state)
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

void lkp_reg_pr(uint8_t src_n, state8080 *state, uint8_t **hi, uint8_t **lo)
{
    switch(src_n) {
        case 0x00:
            *hi = &state->b;
            *lo = &state->c;
            break;
        case 0x01:
            *hi = &state->d;
            *lo = &state->e;
            break;
        case 0x02:
            *hi = &state->h;
            *lo = &state->l;
            break;
        case 0x03:
            *lo = (uint8_t *) &state->sp;
            *hi = *lo + 1;
            break;

        default:
            fprintf(stderr, "Unkown source: %02x lookup_register_pair\n", src_n);
            exit(1);
    }
}

void lkp_reg_pr_psw(uint8_t src_n, state8080 *state, uint8_t **hi, uint8_t **lo)
{
    switch(src_n) {
        case 0x00:
            *hi = &state->b;
            *lo = &state->c;
            break;
        case 0x01:
            *hi = &state->d;
            *lo = &state->e;
            break;
        case 0x02:
            *hi = &state->h;
            *lo = &state->l;
            break;
        case 0x03:
            *hi = &state->a;
            *lo = (uint8_t *) &state->cf;
            break;

        default:
            fprintf(stderr, "Unkown source: %02x lookup_register_pair\n", src_n);
            exit(1);
    }
}

void jmp(state8080 *state)
{
    state->pc = (uint16_t) ((state->memory[state->pc + 2] << 8) + state->memory[state->pc + 1]);
}

void pop_sp(state8080 *state)
{
    state->pc = (uint16_t) (state->memory[state->sp + 1] << 8) + state->memory[state->sp];
    state->sp += 2;
}

void push_sp(state8080 *state)
{
    state->memory[state->sp - 1] = (uint8_t) (state->pc >> 8);
    state->memory[state->sp - 2] = (uint8_t) state->pc;
    state->sp -= 2;
}

bool check_cf_con(uint8_t con, state8080 *state)
{
    switch (con) {
        case 0x00: // ?nz
            if (!state->cf.z) return true;
        case 0x01: // ?z
            if (state->cf.z) return true;
        case 0x02: // ?nc
            if (!state->cf.cy) return true;
        case 0x03: // ?c
            if (state->cf.cy) return true;
        case 0x04: // ?po
            if (!state->cf.p) return true;
        case 0x05: // ?pe
            if (state->cf.p) return true;
        case 0x06: // ?p
            if (!state->cf.s) return true;
        case 0x07: // ?m
            if (state->cf.s) return true;
        default:
            return false;
    }
}

void set_szp(state8080 *state, uint8_t result)
{
    state->cf.s = (result & 0x80) == 0x80;
    state->cf.z = result == 0x00;
    state->cf.p = is_even_parity(result);
}

void do_arith_op(uint8_t op_n, uint8_t src_val , state8080 *state)
{
    uint16_t buffer;
    switch (op_n) {
        case 0x00: // add (a <- a + source)
            buffer = (uint16_t) (state->a + src_val);
            state->a = (uint8_t) buffer;
            state->cf.cy = buffer > 0xff;
            set_szp(state, state->a);
            break;

        case 0x01: // adc (a <- a + source + cy)
            buffer = (uint16_t) (state->a + src_val + state->cf.cy);

            state->a = (uint8_t) buffer;
            state->cf.cy = buffer > 0xff;
            set_szp(state, state->a);
            break;

        case 0x02: // sub (a <- a - source)
            buffer = state->a + (uint8_t) ~src_val + 0x01;

            state->a = (uint8_t) buffer;
            state->cf.cy = !(buffer > 0xff);
            set_szp(state, state->a);
            break;
            
        case 0x03: // sbb (a <- a - source)
            buffer = state->a + (uint8_t) ~(src_val + state->cf.cy) + 0x01;

            state->a = (uint8_t) buffer;
            state->cf.cy = !(buffer > 0xff);
            set_szp(state, state->a);
            break;

        case 0x04: // ana (a <- a & source)
            state->a = (uint8_t) (state->a & src_val);
            state->cf.cy = false;
            set_szp(state, state->a);
            break;

        case 0x05: // xra (a <- a ^ source)
            state->a = (uint8_t) (state->a ^ src_val);
            state->cf.cy = false;
            set_szp(state, state->a);
            break;

        case 0x06: // ora (a <- a | source)
            state->a = (uint8_t) (state->a | src_val);
            state->cf.cy = false;
            set_szp(state, state->a);
            break;

        case 0x07: // cmp (a < source)
            {
                buffer = state->a + (uint8_t) ~src_val + 0x01;

                uint8_t result = (uint8_t) buffer;
                state->cf.cy = !(buffer > 0xff);

                set_szp(state, result);
                break;
            }
    }
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
        uint8_t source_value = *lkp_reg(source, state);
        uint8_t destination = (opcode >> 3) & 0x07;
        uint8_t *destination_pointer = lkp_reg(destination, state);

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
        uint8_t *destination_pointer = lkp_reg(destination, state);

        if (destination_pointer == NULL) {
            return false;
        }

        *destination_pointer = state->memory[state->pc + 1];
        opbytes = 2;
    }

    // inr
    if ((opcode & 0xc7) == 0x04) {
        uint8_t destination = opcode >> 3 & 0x07;
        uint8_t *destination_pointer = lkp_reg(destination, state);
        
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
        uint8_t *destination_pointer = lkp_reg(destination, state);
        
        if (destination_pointer == NULL) {
            return false;
        }

        buffer = *destination_pointer - 1;
        *destination_pointer = (uint8_t) buffer;
        set_szp(state, *destination_pointer);
    }

    // add/adc/sub/sbb/ana/xra/ora/cmp
    if ((opcode & 0xc0) == 0x80) {
        uint8_t src_reg = opcode & 0x07;
        uint8_t src_val = *lkp_reg(src_reg, state);
        uint8_t op_n = (opcode >> 3) & 0x03;

        do_arith_op(op_n, src_val, state);
    }

    // adi/aci/sui/sbi/ani/xri/ori/cpi
    if ((opcode & 0xc7) == 0xc6) {
        uint8_t src_val = state->memory[state->pc + 1];
        uint8_t op_n = (opcode >> 3) & 0x03;

        do_arith_op(op_n, src_val, state);
        opbytes = 2;
    }

    // dad
    if ((opcode & 0xcf) == 0x09) {
        uint8_t *hi, *lo;
        uint8_t src_num = (uint8_t) ((opcode & 0x30) >> 4);

        lkp_reg_pr(src_num, state, &hi, &lo);

        uint32_t src = (uint32_t) ((*hi << 8) + *lo);

        buffer = (uint32_t) ((state->h << 8) + state->l) + src;
        state->h = (uint8_t) (buffer >> 8);
        state->l = (uint8_t) buffer;

        state->cf.cy = buffer > 0xffff;
    }

    // lxi
    if ((opcode & 0xcf) == 0x01) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        lkp_reg_pr(src, state, &hi, &lo);

        *hi = state->memory[state->pc + 2];
        *lo = state->memory[state->pc + 1];
        opbytes = 3;
        printf("lxi\n");
    }
    
    // inx
    if ((opcode & 0xcf) == 0x03) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        lkp_reg_pr(src, state, &hi, &lo);
        
        uint16_t buf = *lo + 1;
        *lo = (uint8_t) buf;
        *hi = (uint8_t) (*hi + (buf >> 8));
    }

    // dcx
    if ((opcode & 0xcf) == 0x0b) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        lkp_reg_pr(src, state, &hi, &lo);

        uint16_t buf = *lo - 1;
        *lo = (uint8_t) buf;
        *hi = (uint8_t) (*hi + (buf >> 8));
    }

    // sta/lda/shld/lhld
    if ((opcode & 0xe7) == 0x22) {
        uint8_t op_n = (opcode >> 3) & 0x03;
        uint8_t hi = state->memory[state->pc + 2];
        uint8_t lo = state->memory[state->pc + 1];
        uint16_t addr = (uint16_t) ((hi << 8) + lo);

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

    // rnz/rz/rnc/rc/rpo/rpe/rp/rm
    if ((opcode & 0xc7) == 0xc0) {
        uint8_t n = (opcode >> 3) & 0x03;
        bool jump = check_cf_con(n, state);

        if (jump) pop_sp(state);
    }
    
    // jnz/jz/jnc/jc/jpo/jpe/jp/jm
    if ((opcode & 0xc7) == 0xc2) {
        uint8_t n = (opcode >> 3) & 0x03;
        bool jump = check_cf_con(n, state);

        if (jump) jmp(state);
        opbytes = 3;
    }

    // cnz/cz/cnc/cc/cpo/cpe/cp/cm
    if ((opcode & 0xc7) == 0xc4) {
        uint8_t n = (opcode >> 3) & 0x03;
        bool jump = check_cf_con(n, state);

        if (jump) {
            push_sp(state);
            jmp(state);
        }
        opbytes = 3;
    }

    // pop
    if ((opcode & 0xcf) == 0xc1) {
        uint8_t src = (uint8_t) ((opcode >> 4) & 0x03);
        uint8_t *hi, *lo;
        lkp_reg_pr_psw(src, state, &hi, &lo);

        *hi = state->memory[state->sp + 1];
        *lo = state->memory[state->sp];
        printf("%02x\n", state->memory[state->sp]);
        printf("%02x\n", state->memory[state->sp + 1]);
        state->sp += 2;
    }
    
    // push
    if ((opcode & 0xcf) == 0xc5) {
        uint8_t dst = (uint8_t) ((opcode >> 4) & 0x03);
        uint8_t *hi, *lo;
        lkp_reg_pr_psw(dst, state, &hi, &lo);

        state->memory[state->sp - 1] = *hi;
        state->memory[state->sp - 2] = *lo;
        printf("%02x\n", state->memory[state->sp - 1]);
        printf("%02x\n", state->memory[state->sp - 2]);
        state->sp -= 2;
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
        case 0x27: // daa
            {
                uint8_t b_l = (uint8_t) state->a & 0x0f;
                uint8_t b_h = (uint8_t) ((state->a & 0xf0) >> 4);

                if (state->cf.ac || b_l > 0x09) {
                    b_l += 0x06;
                    state->cf.ac = b_l > 0x0f;
                }

                if (state->cf.cy || b_h > 0x09) {
                    b_h += 0x06;
                    state->cf.cy = b_h > 0x0f;
                }

                state->a = (uint8_t) ((b_h << 4) + b_l);
                set_szp(state, state->a);
            }
            break;
        // case 0x28: nop
        // case 0x29: dad h
        // case 0x2a: lhld adr    
        // case 0x2b: dcx h
        // case 0x2c: inr l
        // case 0x2d: dcr l
        // case 0x2e: mvi l, d8
        case 0x2f: // cma
            state->a = ~state->a;
            break;
        // case 0x30: nop
        // case 0x31: lxi sp
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
        // case 0xc0: rnz
        // case 0xc1: pop b
        // case 0xc2: jnz
        case 0xc3: // jmp
            jmp(state);
            opbytes = 3;
            break;
        // case 0xc4: cnz
        // case 0xc5: push b
        // case 0xc6: adi

        // case 0xc8: rz
        case 0xc9: // ret
            state->pc = (uint16_t) ((state->memory[state->sp + 1] << 8) + state->memory[state->sp]);
            state->sp += 2;
            break;
        // case 0xca: jz
        // case 0xcb: nop
        // case 0xcc: cz
        case 0xcd: // call
            push_sp(state);
            jmp(state);
            break;
        // case 0xce: aci

        // case 0xd0: rnc
        // case 0xd1: pop d
        // case 0xd2: jnc
        
        // case 0xd4: cnc
        // case 0xd5: push d
        // case 0xd6: sui

        // case 0xd8: rc 
        // case 0xd9: nop
        // case 0xda: jc

        // case 0xdc: cc
        // case 0xdd: nop
        // case 0xde: sbi

        // case 0xe0: rpo
        // case 0xe1: pop h
        // case 0xe2: jpo

        // case 0xe4: cpo
        // case 0xe5: push h
        // case 0xe6: ani

        // case 0xe8: rpe

        // case 0xea: jpe

        // case 0xec: cpe
        // case 0xed: nop
        // case 0xee: xri

        // case 0xf0: rp
        // case 0xf1: pop psw
        // case 0xf2: jpe

        // case 0xf4: cp
        // case 0xf5: push psw
        // case 0xf6: ori

        // case 0xf8: rm

        // case 0xfa: jm

        // case 0xfc: cm
        // case 0xfd: nop
        // case 0xfe: cpi

    
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
        size_t bc = 0;
        // Disassemble 
        while (bc < fsize) {
            bc += disassemble_op8080(state.memory, bc);
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
