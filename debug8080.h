#pragma once

#include "emulator8080.h"

void print_state_pre(const state8080 *state);
void print_state_post(const state8080 *state);
void print_registers8080(const state8080 *state);
void print_condition_flags8080(const state8080 *state);

