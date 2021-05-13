#include "emulator8080.h"
#include "disassembler8080.h"
#include "test8080.h"

static void usage(void)
{
    printf("Usage: emulator8080 [-dt] [file]\n");
    printf("  Emulate an Intel8080 system for a rom file.\n");
    printf("  The following additional options are available:\n");
    printf("  -d\tdisassemble entered rom file\n");
    printf("  -t\trun tests on the emulator (no file argument used)\n");
}

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], "-t") == 0) {
        run_test_cases("test/test_cases.txt");
    } else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 3) {
            printf("Enter file to disassemble\n");
            return 1;
        }

        if (run_disassembler(argv[2])) {
            return 1;
        }
    } else {
        run_emulator(argv[1]);
    }

    return 0;
}

