#pragma once
#include <stdint.h>

uint16_t* assembler_assemble_program(char* assembly);
uint16_t* assembler_assemble_file(char* filename);
uint16_t* assembler_read_bin_file(char* filename);
void assembler_write_bin_file(uint16_t* memory, char* filename);
