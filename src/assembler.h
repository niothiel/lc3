#pragma once
#include <stdint.h>

uint16_t* assembler_assemble_program(char* assembly);
uint16_t* assembler_assemble_file(char* filename);
