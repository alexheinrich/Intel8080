all: disassembler8080 emulator8080
clean:
	rm -f *.o disassembler8080 emulator8080
disassembler8080: disassembler8080.c
	gcc disassembler8080.c -O2 -Wall -Wextra -Wconversion -Wsign-conversion -o disassembler8080
emulator8080: emulator8080.o debug8080.o
	gcc emulator8080.o debug8080.o -o emulator8080
%.o: %.c
	gcc $< -O2 -Wall -Wextra -Wconversion -Wsign-conversion -c -o $@


