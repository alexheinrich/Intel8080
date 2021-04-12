all: emulator8080
clean:
	rm -f *.o emulator8080
emulator8080: main.o emulator8080.o debug8080.o disassembler8080.o utils.o test8080.o
	gcc -g main.o emulator8080.o debug8080.o disassembler8080.o utils.o test8080.o -o emulator8080
%.o: %.c $(wildcard *.h) Makefile
	bear -- gcc $< -Og -g -Wall -Wextra -Wconversion -Wsign-conversion -c -o $@
run_test: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all ./emulator8080 -t
