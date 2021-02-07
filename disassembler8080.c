#include <stdio.h>
#include <stdlib.h>


int Disassembly8080Op(unsigned char *buffer, int pc)
{
        int opbytes = 1;
        printf("%02x ", buffer[pc]);
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
                printf("shld adr, #%02x$02x", buffer[pc+2], buffer[pc+1]);
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
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    printf("%d\n", fsize);
   
    unsigned char *buffer = malloc(fsize);
    
    fread(buffer, sizeof(unsigned char), fsize, f);

    int pc = 0;

    while(pc < fsize) {
        pc += Disassembly8080Op(buffer, pc);
    }
    
    fclose(f);
    return 0;
}
