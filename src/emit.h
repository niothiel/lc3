#pragma once
#include <stdbool.h>
#include <stdint.h>

// Operations
uint16_t emit_NOT(uint16_t dst_register, uint16_t src_register);
uint16_t emit_ADD_imm(uint16_t dst_register, uint16_t src_register, uint16_t value);
uint16_t emit_ADD_reg(uint16_t dst_register, uint16_t src_register, uint16_t src2_register);
uint16_t emit_AND_imm(uint16_t dst_register, uint16_t src_register, uint16_t value);
uint16_t emit_AND_reg(uint16_t dst_register, uint16_t src_register, uint16_t src2_register);

// Data Movement
uint16_t emit_LD(int16_t pc_offset, uint16_t dst_register);
uint16_t emit_ST(uint16_t pc_offset, uint16_t src_register);
uint16_t emit_LDI(int16_t pc_offset, uint16_t dst_register);
uint16_t emit_STI(uint16_t pc_offset, uint16_t src_register);
uint16_t emit_LDR(int16_t pc_offset, uint16_t dst_register, uint16_t base_register);
uint16_t emit_STR(uint16_t pc_offset, uint16_t src_register, uint16_t base_register);
uint16_t emit_LEA(int16_t pc_offset, uint16_t dst_register);

// Control
uint16_t emit_TRAP(uint8_t trap_code);
uint16_t emit_BR(int16_t pc_offset, bool positive, bool zero, bool negative);
uint16_t emit_JMP(uint16_t src_register);
