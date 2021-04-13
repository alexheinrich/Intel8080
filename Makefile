GCC := GCC
C_FLAGS := -Og -g -Wall -Wextra -Wconversion -Wsign-conversion

SRC := main.c emulator8080.c debug8080.c disassembler8080.c utils8080.c test8080.c
OBJ := $(SRC:.c=.o)
OBJ_P := $(OBJ:%=build/%)

build/%.o: src/%.c $(wildcard *.h) Makefile
	bear -- gcc $(C_FLAGS) -c -o $@ $<  
emulator8080: $(OBJ_P)
	gcc -g $^ -o emulator8080
run_test: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all ./emulator8080 -t
all: emulator8080
clean:
	rm -rf build/*.o *.dSYM/ emulator8080
