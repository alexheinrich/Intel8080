SDL2 := /opt/sdl2-2.0.5
SDL2_INC := $(SDL2)/include
SDL2_LIB := $(SDL2)/lib
LIBS =	$(SDL2_LIB)/libSDL2.a \
		-framework AppKit \
		-framework AudioToolbox \
		-framework Carbon \
		-framework CoreAudio \
		-framework CoreVideo \
		-framework ForceFeedback \
		-framework IOKit \
		-l iconv

GCC := gcc
C_FLAGS := -Og -g -Wall -Wextra -Wconversion -Wsign-conversion -I $(SDL2_INC)

SRC := main.c emulator8080.c debug8080.c disassembler8080.c utils8080.c test8080.c shift_register.c video_driver.c
OBJ := $(SRC:.c=.o)
OBJ_P := $(OBJ:%=build/%)

all: emulator8080
build/%.o: src/%.c $(wildcard src/*.h) Makefile
	$(GCC) $(C_FLAGS) -c -o $@ $<  
emulator8080: $(OBJ_P)
	$(GCC) -g $(LIBS) $^ -o emulator8080
run_invaders: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all ./emulator8080 rom/invaders
run_disassembler: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all ./emulator8080 -d rom/invaders
run_test: emulator8080
	valgrind --leak-check=full --show-leak-kinds=all ./emulator8080 -t
clean:
	rm -rf build/*.o *.dSYM/ emulator8080
