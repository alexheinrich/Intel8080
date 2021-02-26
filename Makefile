all: disassembler8080 emulator8080
disassembler8080: disassembler8080.c
	gcc disassembler8080.c -O2 -Wall -Wextra -Wconversion -Wsign-conversion -o disassembler8080
emulator8080: emulator8080.c
	gcc emulator8080.c debug8080.c -O2 -Wall -Wextra -Wconversion -Wsign-conversion -o emulator8080
