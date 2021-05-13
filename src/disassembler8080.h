#pragma once

#include <stddef.h>
#include <stdint.h>

int32_t run_disassembler(char *rom);
size_t disassemble_op8080(unsigned char *buffer, size_t pc);
