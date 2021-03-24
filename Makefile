all: emulator8080
clean:
	rm -f *.o emulator8080
emulator8080: emulator8080.o debug8080.o disassembler8080.o 
	gcc -g emulator8080.o debug8080.o disassembler8080.o -o emulator8080
%.o: %.c Makefile
	gcc $< -Og -g -Wall -Wextra -Wconversion -Wsign-conversion -c -o $@

