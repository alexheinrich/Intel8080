SDL2 := /opt/sdl2
SDL2_INC := $(SDL2)/include
SDL2_LIB := $(SDL2)/lib

LIBS =	$(SDL2_LIB)/libSDL2.a $(SDL2_LIB)/libSDL2_mixer.a $(SDL2_LIB)/libSDL2_image.a

GCC := gcc
C_FLAGS := -Og -g -Wall -Wextra -Wconversion -Wsign-conversion -fno-strict-aliasing \
		   -I $(SDL2_INC)

SRC := main.c emulator8080.c debug8080.c disassembler8080.c utils8080.c \
	   test8080.c shift_register.c sdl.c
OBJ := $(SRC:.c=.o)
OBJ_P := $(OBJ:%=build/%)

all: emulator8080
build/%.o: src/%.c $(wildcard src/*.h) Makefile
	$(GCC) $(C_FLAGS) -c -o $@ $<  
emulator8080: $(OBJ_P) $(LIBS)
	$(GCC) -pthread -o emulator8080 $^ -ldl -lm 
run_invaders: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=./misc/valgrind.supp ./emulator8080 rom/invaders
run_disassembler: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=./misc/valgrind.supp ./emulator8080 -d rom/invaders
run_test: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=./misc/valgrind.supp ./emulator8080 -t
create_log: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --suppressions=./misc/valgrind.supp --log-file=misc/valgrind.log ./emulator8080 rom/invaders
clean:
	rm -rf build/*.o *.dSYM/ emulator8080
