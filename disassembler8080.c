#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char registers[8] = {'b', 'c', 'd', 'e', 'h', 'l', 'm', 'a'};


size_t Disassembly8080Op(unsigned char *buffer, size_t pc)
{
    size_t opbytes = 1;
    printf("%02x ", buffer[pc]);

    if ((buffer[pc] & 0xc0) == 0x40) {
        uint8_t source = buffer[pc] & 0x07;
        uint8_t destination = (buffer[pc] >> 3) & 0x07;
       
        if (source == 0x06 && destination == 0x06) {
            printf("hlt");
        } else {
            printf("mov  %c, %c", registers[destination], registers[source]);
        }
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
        case 0x04:
            printf("inr  b");
            break;
        case 0x05:
            printf("dcr  b");
            break;
        case 0x06:
            printf("mvi  b, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0x07:
            printf("rlc");
            break;
        case 0x08:
            break;
        case 0x09:
            printf("dad  b");
            break;
        case 0x0a:
            printf("ldax b");
            break;
        case 0x0b:
            printf("dcx  b");
            break;
        case 0x0c:
            printf("inr  c");
            break;
        case 0x0d:
            printf("dcr  c");
            break;
        case 0x0e:
            printf("mvi  c, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
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
        case 0x14:
            printf("inr  d");
            break;
        case 0x15:
            printf("dcr  d");
            break;
        case 0x16:
            printf("mvi  d, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
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
        case 0x1c:
            printf("inr  e");
            break;
        case 0x1d:
            printf("dcr  e");
            break;
        case 0x1e:
            printf("mvi  e, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
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
        case 0x24:
            printf("inr  h");
            break;
        case 0x25:
            printf("dcr  h");
            break;
        case 0x26:
            printf("mvi  h, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
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
        case 0x2c:
            printf("inr  l");
            break;
        case 0x2d:
            printf("dcr  l");
            break;
        case 0x2e:
            printf("mvi  l, #%02x", buffer[pc+1]);
            opbytes = 2;
            break;
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
        case 0x34:
            printf("inr  m");
            break;
        case 0x35:
            printf("dcr  m");
            break;
        case 0x36:
            printf("mvi  m, #%02x%02x", buffer[pc+2], buffer[pc+1]);
            break;

        case 0x37:
            printf("stc");
            break;
        case 0x38:
            break;
        case 0x39:
            printf("dad  sp");
            break;
        case 0x3a:
            printf("lda  #%0x02%0x02", buffer[pc+2], buffer[pc+1]);
            opbytes = 3;
            break;
        case 0x3b:
            printf("dcx  sp");
            break;
        case 0x3c:
            printf("inr  a");
            break;
        case 0x3d:
            printf("dcr  a");
            break;
        case 0x3e:
            printf("mvi  a, #%0x02", buffer[pc+1]);
            opbytes = 2;
            break;
        case 0x3f:
            printf("cmc");
            break;
        // 0x40 - 0x7f mov
        case 0x80:
            printf("add  b");
            break;
        case 0x81:
            printf("add  c");
            break;
        case 0x82:
            printf("add  d");
            break;
        case 0x83:
            printf("add  e");
            break;
        case 0x84:
            printf("add  h");
            break;
        case 0x85:
            printf("add  l");
            break;
        case 0x86:
            printf("add  m");
            break;
        case 0x87:
            printf("add  a");
            break;
        case 0x88:
            printf("adc  b");
            break;
        case 0x89:
            printf("adc  c");
            break;
        case 0x8a:
            printf("adc  d");
            break;
        case 0x8b:
            printf("adc  e");
            break;
        case 0x8c:
            printf("adc  h");
            break;
        case 0x8d:
            printf("adc  l");
            break;
        case 0x8e:
            printf("adc  m");
            break;
        case 0x8f:
            printf("adc  a");
            break;
        case 0x90:
            printf("sub  b");
            break;
        case 0x91:
            printf("sub  c");
            break;
        case 0x92:
            printf("sub  d");
            break;
        case 0x93:
            printf("sub  e");
            break;
        case 0x94:
            printf("sub  h");
            break;
        case 0x95:
            printf("sub  l");
            break;
        case 0x96:
            printf("sub  m");
            break;
        case 0x97:
            printf("sub  a");
            break;
        case 0x98:
            printf("sbb  b");
            break;
        case 0x99:
            printf("sbb  c");
            break;
        case 0x9a:
            printf("sbb  d");
            break;
        case 0x9b:
            printf("sbb  e");
            break;
        case 0x9c:
            printf("sbb  h");
            break;
        case 0x9d:
            printf("sbb  l");
            break;
        case 0x9e:
            printf("sbb  m");
            break;
        case 0x9f:
            printf("sbb  a");
            break;
        case 0xa0:
            printf("ana  b");
            break;
        case 0xa1:
            printf("ana  c");
            break;
        case 0xa2:
            printf("ana  d");
            break;
        case 0xa3:
            printf("ana  e");
            break;
        case 0xa4:
            printf("ana  h");
            break;
        case 0xa5:
            printf("ana  l");
            break;
        case 0xa6:
            printf("ana  m");
            break;
        case 0xa7:
            printf("ana  a");
            break;
        case 0xa8:
            printf("xra  b");
            break;
        case 0xa9:
            printf("xra  c");
            break;
        case 0xaa:
            printf("xra  d");
            break;
        case 0xab:
            printf("xra  e");
            break;
        case 0xac:
            printf("xra  h");
            break;
        case 0xad:
            printf("xra  l");
            break;
        case 0xae:
            printf("xra  m");
            break;
        case 0xaf:
            printf("xra  a");
            break;
        case 0xb0:
            printf("ora  b");
            break;
        case 0xb1:
            printf("ora  c");
            break;
        case 0xb2:
            printf("ora  d");
            break;
        case 0xb3:
            printf("ora  e");
            break;
        case 0xb4:
            printf("ora  h");
            break;
        case 0xb5:
            printf("ora  l");
            break;
        case 0xb6:
            printf("ora  m");
            break;
        case 0xb7:
            printf("ora  a");
            break;
        case 0xb8:
            printf("cmp  b");
            break;
        case 0xb9:
            printf("cmp  c");
            break;
        case 0xba:
            printf("cmp  d");
            break;
        case 0xbb:
            printf("cmp  e");
            break;
        case 0xbc:
            printf("cmp  h");
            break;
        case 0xbd:
            printf("cmp  l");
            break;
        case 0xbe:
            printf("cmp  m");
            break;
        case 0xbf:
            printf("cmp  a");
            break;
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

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("fopen() failed to open: %s. Errno: %s.\n", argv[1], strerror(errno));
        exit(1);
    }

    int fseek_end_status = fseek(f, 0L, SEEK_END);
    if (fseek_end_status < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }
    
    size_t fsize = (size_t) ftell(f);
    if (fsize < 0) {
        printf("ftell() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    int fseek_start_status = fseek(f, 0L, SEEK_SET);
    if (fseek_start_status < 0) {
        printf("fseek() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    unsigned char *buffer = malloc(fsize);
    if (buffer == NULL) {
        printf("malloc() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    size_t fread_status = fread(buffer, sizeof(unsigned char), fsize, f);
    if (fread_status != (sizeof(unsigned char) * fsize)) {
        printf("%zu", fread_status);
        printf("fread() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    size_t pc = 0;

    while(pc < fsize) {
        pc += Disassembly8080Op(buffer, pc);
    }

    int fclose_status = fclose(f);
    if (fclose_status != 0) {
        printf("fclose() failed. Errno: %s.\n", strerror(errno));
        exit(1);
    }

    return 0;
}
