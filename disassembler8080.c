#include <stdio.h>
#include <stdlib.h>


int Disassembly8080Op(unsigned char *buffer, int pc)
{
    for (int i = 0; i < pc; i++) {
        printf("%04x ", buffer[i]);
        switch(buffer[i]) {
            case 0x00: 
                printf("NOP");
                break;
            case 0x01:
                printf("LXIB,#$%02x%02x", buffer[i+2], buffer[i+1]);
                i = i + 2;
                break;

            default:
                break;
        }
        printf("\n");

    }

    return 1;
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

    int pc = fsize;

    //while(pc < fsize) {
    //    pc += Disassembly8080Op(buffer, pc);
    //}
    pc = Disassembly8080Op(buffer, pc);
    fclose(f);
    return 0;
}
