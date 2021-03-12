#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char registers[8] = {'b', 'c', 'd', 'e', 'h', 'l', 'm', 'a'};

void interpret654(char *opcode_name, uint8_t opcode)
{
    uint8_t destination = (opcode >> 3) & 0x07;
    printf("%s  %c", opcode_name, registers[destination]);
}

void interpret210(char *opcode_name, uint8_t opcode)
{
    uint8_t source = opcode & 0x07;
    printf("%s  %c", opcode_name, registers[source]);
}

size_t disassemble_op8080(unsigned char *buffer, size_t pc)
{
    size_t opbytes = 1;
    printf("%04zx %02x ", pc, buffer[pc]);

    // mov (and hlt)
    if ((buffer[pc] & 0xc0) == 0x40) {
        uint8_t source = buffer[pc] & 0x07;
        uint8_t destination = (buffer[pc] >> 3) & 0x07;
       
        if (source == 0x06 && destination == 0x06) {
            printf("hlt");
        } else {
            printf("mov  %c, %c", registers[destination], registers[source]);
        }
    }

    // mvi
    if ((buffer[pc] & 0xc7) == 0x06) {
        uint8_t destination = (buffer[pc] >> 3) & 0x07;
        printf("mvi  %c, #%02x", registers[destination], buffer[pc+1]);
        opbytes = 2;
    }

    // inr
    if ((buffer[pc] & 0xc7) == 0x04) {
        interpret654("inr", buffer[pc]);
    }

    // dcr
    if ((buffer[pc] & 0xc7) == 0x05) {
        interpret654("dcr", buffer[pc]);
    }

    // add
    if ((buffer[pc] & 0xf8) == 0x80) {
        interpret210("add", buffer[pc]);
    }

    // adc
    if ((buffer[pc] & 0xf8) == 0x88) {
        interpret210("adc", buffer[pc]);
    }

    // sub
    if ((buffer[pc] & 0xf8) == 0x90) {
        interpret210("sub", buffer[pc]);
    }

    // sbb
    if ((buffer[pc] & 0xf8) == 0x98) {
        interpret210("sbb", buffer[pc]);
    }

    // ana
    if ((buffer[pc] & 0xf8) == 0xa0) {
        interpret210("ana", buffer[pc]);
    }

    // xra
    if ((buffer[pc] & 0xf8) == 0xa8) {
        interpret210("xra", buffer[pc]);
    }

    // ora
    if ((buffer[pc] & 0xf8) == 0xb0) {
        interpret210("ora", buffer[pc]);
    }

    // cmp
    if ((buffer[pc] & 0xf8) == 0xb8) {
        interpret210("cmp", buffer[pc]);
    }
    
    // rst
    if ((buffer[pc] & 0xc7) == 0xc7) {
        printf("rst  %02x", (buffer[pc] >> 3) & 0x07);
    }



    switch(buffer[pc]) {
        case 0x00: 
            printf("nop");
            break;
        case 0x01:
            printf("lxi  b, #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x02:
            printf("stax b");
            break;
        case 0x03:
            printf("inx  b");
            break;
        // case 0x04: inr b
        // case 0x05: dcr b
        // case 0x06: mvi b
        case 0x07:
            printf("rlc");
            break;
        // case 0x08: nop
        case 0x09:
            printf("dad  b");
            break;
        case 0x0a:
            printf("ldax b");
            break;
        case 0x0b:
            printf("dcx  b");
            break;
        // case 0x0e: mvi c
        case 0x0f:
            printf("rrc");
            break;
        case 0x10:
            break;
        case 0x11:
            printf("lxi  d, #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x12:
            printf("stax  d");
            break;
        case 0x13:
            printf("inx  d");
            break;
        // case 0x16: mvi d
        case 0x17:
            printf("ral");
            break;
        case 0x18:
            break;
        case 0x19:
            printf("dad  d");
            break;
        case 0x1a:
            printf("ldax d");
            break;
        case 0x1b:
            printf("dcx  d");
            break;
        // case 0x1e: mvi e
        case 0x1f:
            printf("rar");
            break;
        case 0x20:
            printf("rim");
            break;
        case 0x21:
            printf("lxi  h, #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x22:
            printf("shld #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x23:
            printf("inx  h");
            break;
        // case 0x26: mvi h
        case 0x27:
            printf("daa");
            break;
        case 0x28:
            break;
        case 0x29:
            printf("dad  h");
            break;
        case 0x2a:
            printf("lhld #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 2;
            break;
        case 0x2b:
            printf("dcx  h");
            break;
        // case 0x2e: mvi l
        case 0x2f:
            printf("cma");
            break;
        case 0x30:
            printf("sim");
            break;
        case 0x31:
            printf("lxi  sp, #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x32:
            printf("sta  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x33:
            printf("inx  sp");
            break;
        // case 0x36: mvi m
        case 0x37:
            printf("stc");
            break;
        case 0x38:
            break;
        case 0x39:
            printf("dad  sp");
            break;
        case 0x3a:
            printf("lda  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x3b:
            printf("dcx  sp");
            break;
        // case 0x3e: mvi a
        case 0x3f:
            printf("cmc");
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
        case 0xc0:
            printf("rnz");
            break;
        case 0xc1:
            printf("pop  b");
            break;
        case 0xc2:
            printf("jnz  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xc3:
            printf("jmp  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xc4:
            printf("cnz  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xc5:
            printf("push  b");
            break;
        case 0xc6:
            printf("adi  #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xc7:
            printf("rst  0");
            break;
        case 0xc8:
            printf("rz");
            break;
        case 0xc9:
            printf("ret");
            break;
        case 0xca:
            printf("jz  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xcb:
            break;
        case 0xcc:
            printf("cz  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xcd:
            printf("call #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xce:
            printf("aci  #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xcf:
            printf("rci  1");
            break;
        case 0xd0:
            printf("rnc");
            break;
        case 0xd1:
            printf("pop  d");
            break;
        case 0xd2:
            printf("jnc  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xd3:
            printf("out  #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xd4:
            printf("cnc  #%02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xd5:
            printf("push  d");
            break;
        case 0xd6:
            printf("sui  #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xd7:
            printf("rst  2");
            break;
        case 0xd8:
            printf("rc");
            break;
        case 0xd9:
            break;
        case 0xda:
            printf("jc  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xdb:
            printf("in  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xdc:
            printf("cc  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xdd:
            break;
        case 0xde:
            printf("sbi  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xdf:
            printf("rst  3");
            break;
        case 0xe0:
            printf("rpo");
            break;
        case 0xe1:
            printf("pop  h");
            break;
        case 0xe2:
            printf("jpo  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xe3:
            printf("xthl");
            break;
        case 0xe4:
            printf("cop  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xe5:
            printf("push  h");
            break;
        case 0xe6:
            printf("ani  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xe7:
            printf("rst  4");
            break;
        case 0xe8:
            printf("rpe");
            break;
        case 0xe9:
            printf("pchl");
            break;
        case 0xea:
            printf("jpe  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xeb:
            printf("xchg");
            break;
        case 0xec:
            printf("cpe  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xed:
            break;
        case 0xee:
            printf("xri  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xef:
            printf("rst  5");
            break;
        case 0xf0:
            printf("rp");
            break;
        case 0xf1:
            printf("pop  psw");
            break;
        case 0xf2:
            printf("jp  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xf3:
            printf("di");
            break;
        case 0xf4:
            printf("cp  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xf5:
            printf("pop  psw");
            break;
        case 0xf6:
            printf("ori  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xf7:
            printf("rst  6");
            break;
        case 0xf8:
            printf("rm");
            break;
        case 0xf9:
            printf("sphl");
            break;
        case 0xfa:
            printf("jm  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xfb:
            printf("ei");
            break;
        case 0xfc:
            printf("cm  %02x%02x", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0xfd:
            break;
        case 0xfe:
            printf("cpi  %02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0xff:
            printf("rst  7");
            break;
        default:
            break;
    }
    printf("\n");


    return opbytes;
}

