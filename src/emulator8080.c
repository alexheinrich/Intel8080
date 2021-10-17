#include "emulator8080.h"
#include "debug8080.h"
#include "shift_register.h"
#include "utils8080.h"
#include "sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define SCREEN_OFFSET 0x2400
#define SCREEN_W_ORIG 256
#define SCREEN_H_ORIG 224
#define SCREEN_W SCREEN_H_ORIG
#define SCREEN_H SCREEN_W_ORIG

uint8_t ports[PORT_NUM] = {0x00};

static uint8_t get_in(uint8_t port)
{
    switch (port) {
        case 0x01:
            return ports[port];
        case 0x03:
            return sreg_get_val();
        default:
            return 0x00;
    }
}

static void wr_out(uint8_t port, uint8_t val)
{
    switch (port) {
        case 0x02: 
            //printf("Write. Port %u, Value %02x\n", port, val);
            sreg_set_shift(val);
            break;
        case 0x03: {
                static uint8_t last_val = 0;

                if ((val & 0x01) & !(last_val & 0x01)) {
                    loop_sound(0);
                } else if ((!(val & 0x01)) & (last_val & 0x01)) {
                    stop_loop(0);
                }

                if (((val >> 1) & 0x01) & !((last_val >> 1) & 0x01)) {
                    play_sound(1);
                }

                if (((val >> 2) & 0x01) & !((last_val >> 2) & 0x01)) {
                    play_sound(2);
                }

                if (((val >> 3) & 0x01) & !((last_val >> 3) & 0x01)) {
                    play_sound(3);
                }

                if (((val >> 4) & 0x01) & !((last_val >> 4) & 0x01)) {
                    play_sound(9);
                }

                last_val = val;
            }

            break;
        case 0x04:
            //printf("Write. Port %u, Value %02x\n", port, val);
            sreg_push_val(val);
            break;
        case 0x05: {
                static uint8_t last_val = 0x00;
                if (((val >> 0) & 0x01) && !((last_val >> 0) & 0x01)) {
                    play_sound(4);
                }
               
                if (((val >> 1) & 0x01) && !((last_val >> 1) & 0x01)) {
                    play_sound(5);
                }
                
                if (((val >> 2) & 0x01) && !((last_val >> 2) & 0x01)) {
                    play_sound(6);
                }

                if (((val >> 3) & 0x01) && !((last_val >> 3) & 0x01)) {
                    play_sound(7);
                }
                
                if (((val >> 4) & 0x01) && !((last_val >> 4) & 0x01)) {
                    play_sound(8);
                }
                last_val = val;
            }

            break;
        default:
            break;
    }
}

static bool par_even(uint8_t num)
{
    uint32_t t1 = num ^ (num >> 4);
    uint32_t t2 = t1 ^ (t1 >> 2);
    uint32_t t3 = t2 ^ (t2 >> 1);

    return !(t3 & 0x01);
}

uint8_t dummy = 0x00;

static uint8_t *get_reg(uint8_t src_num, state8080 *state)
{
    switch (src_num) {
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
                uint16_t addr = (uint16_t) ((state->h << 8) + state->l);
                return &state->memory[addr];
            }
        case 7:
            return &state->a;
        default:
            printf("Error: Undefined Register.\n");
            exit(1);
    }

}

static void get_reg_pr(uint8_t **hi, uint8_t **lo, uint8_t src_num, state8080 *state)
{
    switch(src_num) {
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
        default:
            fprintf(stderr, "Unkown source: %02x get_regp\n", src_num);
            exit(1);
    }
}

static void get_reg_pr_sp(uint8_t **hi, uint8_t **lo, uint8_t src_num, state8080 *state)
{
    if (src_num == 0x03) {
        *lo = (uint8_t *) &state->sp;
        *hi = *lo + 1;
    } else {
        get_reg_pr(hi, lo, src_num, state);
    }
}

static void get_reg_pr_psw(uint8_t **hi, uint8_t **lo, uint8_t src_num, state8080 *state)
{
    if (src_num == 0x03) {
        *hi = &state->a;
        *lo = (uint8_t *) &state->cf;
    } else {
        get_reg_pr(hi, lo, src_num, state);
    }
}

static void jmp(state8080 *state)
{
    state->pc = (uint16_t) ((state->memory[state->pc + 2] << 8) + state->memory[state->pc + 1]);
}

static void pop_pc(state8080 *state)
{
    state->pc = (uint16_t) (state->memory[state->sp + 1] << 8) + state->memory[state->sp];
    state->sp += 2;
}

static void push_pc(state8080 *state, uint8_t opbytes)
{
    uint16_t pc = (uint16_t) (state->pc + opbytes);
    state->memory[state->sp - 1] = (uint8_t) (pc >> 8);
    state->memory[state->sp - 2] = (uint8_t) pc;
    state->sp -= 2;
}

static bool check_cf(uint8_t con, state8080 *state)
{
    switch (con) {
        case 0x00: // ?nz
            if (!state->cf.z) return true;
            break;
        case 0x01: // ?z
            if (state->cf.z) return true;
            break;
        case 0x02: // ?nc
            if (!state->cf.cy) return true;
            break;
        case 0x03: // ?c
            if (state->cf.cy) return true;
            break;
        case 0x04: // ?po
            if (!state->cf.p) return true;
            break;
        case 0x05: // ?pe
            if (state->cf.p) return true;
            break;
        case 0x06: // ?p
            if (!state->cf.s) return true;
            break;
        case 0x07: // ?m
            if (state->cf.s) return true;
            break;
        default:
            fprintf(stderr, "Unkown source: %02x check_cf\n", con);
            exit(1);
    }

    return false;
}

static void set_szp(state8080 *state, uint8_t result)
{
    state->cf.s = (result & 0x80) == 0x80;
    state->cf.z = result == 0x00;
    state->cf.p = par_even(result);
}

static void do_arith_op(uint8_t op_num, uint8_t src_val , state8080 *state)
{
    switch (op_num) {
        case 0x00: // add (a <- a + source)
            {
                uint16_t tmp = (uint16_t) (state->a + src_val);
                state->a = (uint8_t) tmp;
                state->cf.cy = (uint8_t) ((tmp >> 8) & 0x01);
                set_szp(state, state->a);
                break;
            }

        case 0x01: // adc (a <- a + source + cy)
            {
                uint16_t tmp = (uint16_t) (state->a + src_val + state->cf.cy);
                state->a = (uint8_t) tmp;
                state->cf.cy = (uint8_t) ((tmp >> 8) & 0x01);
                set_szp(state, state->a);
                break;
            }

        case 0x02: // sub (a <- a - source)
            {
                uint16_t tmp = (uint16_t) (state->a - src_val);
                state->a = (uint8_t) tmp;
                state->cf.cy = (uint8_t) ((tmp >> 8) & 0x01);
                set_szp(state, state->a);
                break;
            }
            
        case 0x03: // sbb (a <- a - source)
            {
                uint16_t tmp = (uint16_t) (state->a - src_val - state->cf.cy);
                state->a = (uint8_t) tmp;
                state->cf.cy = (uint8_t) ((tmp >> 8) & 0x01);
                set_szp(state, state->a);
                break;
            }

        case 0x04: // ana (a <- a & source)
            {
                state->a = (uint8_t) (state->a & src_val);
                state->cf.cy = 0;
                set_szp(state, state->a);
                break;
            }

        case 0x05: // xra (a <- a ^ source)
            {
                state->a = (uint8_t) (state->a ^ src_val);
                state->cf.cy = 0;
                set_szp(state, state->a);
                break;
            }

        case 0x06: // ora (a <- a | source)
            {
                state->a = (uint8_t) (state->a | src_val);
                state->cf.cy = 0;
                set_szp(state, state->a);
                break;
            }

        case 0x07: // cmp (a < source)
            {
                uint16_t tmp = (uint16_t) (state->a - src_val);
                state->cf.cy = (uint8_t) ((tmp >> 8) & 0x01);
                set_szp(state, (uint8_t) tmp);
                break;
            }
    }
}

bool emulate_op8080(state8080 *state, bool debug)
{
    unsigned char opcode = state->memory[state->pc];
    uint8_t pc_inr = 1;
    
    if (debug) {
        print_state_pre(state);
    }

    // mov (and hlt)
    if ((opcode & 0xc0) == 0x40) {
        uint8_t src = opcode & 0x07;
        uint8_t src_val = *get_reg(src, state);
        uint8_t dst = (opcode >> 3) & 0x07;
        uint8_t *dst_p = get_reg(dst, state);

        if (dst_p == NULL) {
            return false;
        }

        // hlt
        if (src == 0x06 && dst == 0x06) {
            return false;
        }

        *dst_p = src_val;
    }

    // mvi (destination <- source)
    if ((opcode & 0xc7) == 0x06) {
        uint8_t dst = (opcode >> 3) & 0x07;
        uint8_t *dst_p = get_reg(dst, state);

        if (dst_p == NULL) {
            return false;
        }

        *dst_p = state->memory[state->pc + 1];
        pc_inr = 2;
    }

    // inr
    if ((opcode & 0xc7) == 0x04) {
        uint8_t dst = opcode >> 3 & 0x07;
        uint8_t *dst_p = get_reg(dst, state);
        
        if (dst_p == NULL) {
            return false;
        }

        uint16_t buf = *dst_p + 1;
        *dst_p = (uint8_t) buf;
        set_szp(state, *dst_p);
    }
    
    // dcr
    if ((opcode & 0xc7) == 0x05) {
        uint8_t dst = opcode >> 3 & 0x07;
        uint8_t *dst_p = get_reg(dst, state);
        
        if (dst_p == NULL) {
            return false;
        }

        uint16_t buf = *dst_p - 1;
        *dst_p = (uint8_t) buf;
        set_szp(state, *dst_p);
    }

    // add/adc/sub/sbb/ana/xra/ora/cmp
    if ((opcode & 0xc0) == 0x80) {
        uint8_t src_reg = opcode & 0x07;
        uint8_t src_val = *get_reg(src_reg, state);
        uint8_t op_num = (opcode >> 3) & 0x07;

        do_arith_op(op_num, src_val, state);
    }

    // adi/aci/sui/sbi/ani/xri/ori/cpi
    if ((opcode & 0xc7) == 0xc6) {
        uint8_t src_val = state->memory[state->pc + 1];
        uint8_t op_num = (opcode >> 3) & 0x07;

        do_arith_op(op_num, src_val, state);
        pc_inr = 2;
    }

    // dad
    if ((opcode & 0xcf) == 0x09) {
        uint8_t *hi, *lo;
        uint8_t src_num = (uint8_t) ((opcode & 0x30) >> 4);

        get_reg_pr_sp(&hi, &lo, src_num, state);

        uint16_t src = (uint16_t) ((*hi << 8) + *lo);
        uint32_t buf = (uint32_t) ((state->h << 8) + state->l) + src;

        state->h = (uint8_t) (buf >> 8);
        state->l = (uint8_t) buf;

        state->cf.cy = buf > 0xffff;
    }

    // lxi
    if ((opcode & 0xcf) == 0x01) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        get_reg_pr_sp(&hi, &lo, src, state);

        *hi = state->memory[state->pc + 2];
        *lo = state->memory[state->pc + 1];
        
        state->pc += 3;
        return true;
    }
    
    // inx
    if ((opcode & 0xcf) == 0x03) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        get_reg_pr_sp(&hi, &lo, src, state);
        
        uint16_t buf = *lo + 1;
        *lo = (uint8_t) buf;
        *hi = (uint8_t) (*hi + (buf >> 8));
    }

    // dcx
    if ((opcode & 0xcf) == 0x0b) {
        uint8_t *hi, *lo; 
        uint8_t src = (uint8_t) ((opcode & 0x30) >> 4);

        get_reg_pr_sp(&hi, &lo, src, state);

        uint16_t buf = *lo - 1;
        *lo = (uint8_t) buf;
        *hi = (uint8_t) (*hi + (buf >> 8));
    }

    // sta/lda/shld/lhld
    if ((opcode & 0xe7) == 0x22) {
        uint8_t op_num = (opcode >> 3) & 0x03;
        uint8_t hi = state->memory[state->pc + 2];
        uint8_t lo = state->memory[state->pc + 1];
        uint16_t addr = (uint16_t) ((hi << 8) + lo);

        switch (op_num) {
            case 0x00: // shld
                state->memory[addr + 1] = state->h;
                state->memory[addr] = state->l;
                break;
            case 0x01: // lhld
                state->h = state->memory[addr + 1];
                state->l = state->memory[addr];
                break;
            case 0x02: // sta
                state->memory[addr] = state->a;
                break;
            case 0x03: // lda
                state->a = state->memory[addr];
                break;
            default:
                fprintf(stderr, "Invalid op_num: %02x sta/lda/shld/lhld\n", op_num);
                exit(1);
        }

        pc_inr = 3;
    }

    // rnz/rz/rnc/rc/rpo/rpe/rp/rm
    if ((opcode & 0xc7) == 0xc0) {
        uint8_t n = (opcode >> 3) & 0x07;
        bool jump = check_cf(n, state);

        if (jump) {
            pop_pc(state);
            pc_inr = 0;
        }
    }
    
    // jnz/jz/jnc/jc/jpo/jpe/jp/jm
    if ((opcode & 0xc7) == 0xc2) {
        uint8_t n = (opcode >> 3) & 0x07;
        bool jump = check_cf(n, state);

        if (jump) {
            jmp(state);
            pc_inr = 0;
        } else {
            pc_inr = 3;
        }
    }

    // cnz/cz/cnc/cc/cpo/cpe/cp/cm
    if ((opcode & 0xc7) == 0xc4) {
        uint8_t n = (opcode >> 3) & 0x07;
        bool jump = check_cf(n, state);

        if (jump) {
            push_pc(state, 3);
            jmp(state);
            pc_inr = 0;
        } else {
            pc_inr = 3;
        }
    }

    // rst 0-7
    if ((opcode & 0xc7) == 0xc7) {
        push_pc(state, 1);
        state->pc = (uint16_t) (opcode & 0x38);
    }

    // pop
    if ((opcode & 0xcf) == 0xc1) {
        uint8_t src = (uint8_t) ((opcode >> 4) & 0x03);
        uint8_t *hi, *lo;
        get_reg_pr_psw(&hi, &lo, src, state);

        *lo = state->memory[state->sp++];
        *hi = state->memory[state->sp++];
    }
    
    // push
    if ((opcode & 0xcf) == 0xc5) {
        uint8_t dst = (uint8_t) ((opcode >> 4) & 0x03);
        uint8_t *hi, *lo;
        get_reg_pr_psw(&hi, &lo, dst, state);

        state->memory[--state->sp] = *hi;
        state->memory[--state->sp] = *lo;
    }

    switch (opcode) {
        // case 0x00: nop
        case 0x01: // lxi b
            {
                state->b = state->memory[state->pc + 2];
                state->c = state->memory[state->pc + 1];
                pc_inr = 3;
                break;
            }
        case 0x02: // stax b
            {
                uint16_t addr = (uint16_t) ((state->b << 8) + state->c);
                state->memory[addr] = state->a;
                break;
            }
        // case 0x03: inx b
        // case 0x04: inr b
        // case 0x05: dcr b
        // case 0x06: mvi b, d8
        case 0x07: // rlc
            {
                uint16_t buf = state->a >> 7;
                state->a = (uint8_t) ((uint8_t) (state->a << 1) + buf);
                state->cf.cy = buf != 0;
                break;
            }
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
            {
                uint8_t buf = (uint8_t) (state->a << 7);
                state->a = (uint8_t) ((state->a >> 1) + buf);
                state->cf.cy = buf != 0;
                break;
            }
        // case 0x10: nop
        case 0x11: // lxi d
            {
                state->d = state->memory[state->pc + 2];
                state->e = state->memory[state->pc + 1];
                pc_inr = 3;
                break;
            }
        case 0x12: // stax d
            {
                uint16_t addr = (uint16_t) ((state->d << 8) + state->e);
                state->memory[addr] = state->a;
                break;
            }
        // case 0x13: inx d
        // case 0x14: inr d
        // case 0x15: dcr d
        // case 0x16: mvi d, d8
        case 0x17: // ral
            {
                uint8_t buf = state->a >> 7 != 0;
                state->a = (uint8_t) ((uint8_t) (state->a << 1) + state->cf.cy);
                state->cf.cy = (uint8_t) (buf & 0x01);
                break;
            }
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
            {
                uint8_t buf = (uint8_t) (state->a & 0x01);
                state->a = (uint8_t) ((state->a >> 1) + (state->cf.cy << 7));
                state->cf.cy = (uint8_t) (buf & 0x01);
                break;
            }
        // case 0x20: nop
        case 0x21: // lxi h
            {
                state->h = state->memory[state->pc + 2];
                state->l = state->memory[state->pc + 1];
                pc_inr = 3;
                break;
            }
        // case 0x22: shld adr
        // case 0x23: inx h
        // case 0x24: inr h
        // case 0x25: dcr h
        // case 0x26: mvi h, d8
        case 0x27: // daa
            {
                uint8_t lo = (uint8_t) (state->a & 0x0f);

                if (state->cf.ac || lo > 0x09) {
                    state->a += 0x06;
                    state->cf.ac = (state->a & 0x0f) < lo;
                } 

                uint8_t hi = (uint8_t) (state->a & 0xf0);

                if (state->cf.cy || hi > 0x90) {
                    state->a += 0x60;
                    state->cf.cy = (state->a & 0xf0) < hi;
                }

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
            {
                state->a = ~state->a;
                break;
            }
        // case 0x30: nop
        // case 0x31: lxi sp
        // case 0x32: sta adr
        // case 0x33: inx sp
        // case 0x34: inr m
        // case 0x35: dcr m
        // case 0x36: mvi m, d8
        case 0x37: // stc
            {
                state->cf.cy = 1;
                break;
            }
        // case 0x38: nop
        // case 0x39: dad sp
        // case 0x3a: lda adr
        // case 0x3b: dcx sp
        // case 0x3c: inr a
        // case 0x3d: dcr a
        // case 0x3e: mvi a, d8
        case 0x3f: // cmc
            {
                state->cf.cy = !state->cf.cy;
                break;
            }
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
            {
                jmp(state);
                pc_inr = 0;
                break;
            }
        // case 0xc4: cnz
        // case 0xc5: push b
        // case 0xc6: adi
        // case 0xc7: rst 0
        // case 0xc8: rz
        case 0xc9: // ret
            {
                state->pc = state->memory[state->sp++];
                state->pc |= (uint16_t) (state->memory[state->sp++] << 8);
                pc_inr = 0;
                break;
            }
        // case 0xca: jz
        // case 0xcb: nop
        // case 0xcc: cz
        case 0xcd: // call
            {
                push_pc(state, 3);
                jmp(state);
                pc_inr = 0;
                break;
            }
        // case 0xce: aci
        // case 0xcf: rst 1
        // case 0xd0: rnc
        // case 0xd1: pop d
        // case 0xd2: jnc
        case 0xd3: //out
            {
                wr_out(state->memory[state->pc + 1], state->a);
                pc_inr = 2;
                break;
            }
        // case 0xd4: cnc
        // case 0xd5: push d
        // case 0xd6: sui
        // case 0xd7: rst 2
        // case 0xd8: rc 
        // case 0xd9: nop
        // case 0xda: jc
        case 0xdb: // in
            {
                state->a = get_in(state->memory[state->pc + 1]);
                pc_inr = 2;
                break;
            }
        // case 0xdc: cc
        // case 0xdd: nop
        // case 0xde: sbi
        // case 0xdf: rst 3
        // case 0xe0: rpo
        // case 0xe1: pop h
        // case 0xe2: jpo
        // TODO: 0xe3: xthl
        // case 0xe4: cpo
        // case 0xe5: push h
        // case 0xe6: ani
        // case 0xe7: rst 4
        // case 0xe8: rpe
        // TODO: 0xe9: pchl
        // case 0xea: jpe
        case 0xeb: // xchg
            {
                state->h ^= state->d;
                state->d ^= state->h;
                state->h ^= state->d;

                state->l ^= state->e;
                state->e ^= state->l;
                state->l ^= state->e;
                break;
            }
        // case 0xec: cpe
        // case 0xed: nop
        // case 0xee: xri
        // case 0xef: rst 5
        // case 0xf0: rp
        // case 0xf1: pop psw
        // case 0xf2: jpe
        case 0xf3:
            state->interrupts_enabled = 0;
            break;
        // case 0xf4: cp
        // case 0xf5: push psw
        // case 0xf6: ori
        // case 0xf7: rst 6
        // case 0xf8: rm
        // TODO: 0xf9: sphl
        // case 0xfa: jm
        case 0xfb:
            state->interrupts_enabled = 1;
            break;
        // case 0xfc: cm
        // case 0xfd: nop
        // case 0xfe: cpi
        // case 0xff: rst 7
    }

    state->pc += pc_inr;

    if (debug) {
        print_state_post(state);
    }

    return true;
}

static void handle_interrupt(state8080 *state, uint32_t iv)
{
    push_pc(state, 0);
    state->interrupts_enabled = 0;
    state->pc = (uint8_t) (8 * iv);
}

int run_emulator(char *rom)
{
    state8080 state;

    if (load_rom(&state, rom) < 0) {
        return 1;
    }
        
    sdl_init();


    uint32_t t, t_inr = 0;
    uint32_t iv = 0x01;
    uint32_t cyc = 0;

    while (true) {
        if (state.pc == 0x18DC) {
            load_hiscore(&state);
        }

        emulate_op8080(&state, false);

        t = SDL_GetTicks();
        
        if (state.interrupts_enabled && (t - t_inr) > 16) {
            handle_interrupt(&state, iv);
            iv ^= 0x03;
            t_inr = t;

            if (sdl_exec(&state)) {
                break;
            }
        }

        if (cyc > 10000) {
            usleep(20000);
            cyc = 0;
        }

        cyc++;
    }

    sdl_quit();
    save_hiscore(&state);
    unload_rom(&state);

    return 0;
}
