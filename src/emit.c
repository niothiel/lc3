#include "emit.h"
#include "opcode.h"
#include "util.h"

static void check_register_index(uint16_t reg_index)
{
    if (reg_index > 7)
        fatalf("Invalid register index: %d\n", reg_index);
}

uint16_t emit_NOT(uint16_t dst_register, uint16_t src_register)
{
    check_register_index(dst_register);
    check_register_index(src_register);
    uint16_t instruction = NOT << 12;
    instruction |= (dst_register & 0x7) << 6;
    instruction |= (src_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_ADD_imm(uint16_t dst_register, uint16_t src_register, uint8_t value)
{
    check_register_index(dst_register);
    check_register_index(src_register);

    uint16_t instruction = ADD << 12;
    instruction |= (dst_register << 9);
    instruction |= (src_register << 6);
    instruction |= (value & 0x1f);
    return instruction;
}

uint16_t emit_ADD_reg(uint16_t dst_register, uint16_t src_register, uint16_t src2_register)
{
    check_register_index(dst_register);
    check_register_index(src_register);
    check_register_index(src2_register);

    uint16_t instruction = ADD << 12;
    instruction |= (dst_register << 9);
    instruction |= (src_register << 6);
    instruction |= (src2_register);
    return instruction;
}

uint16_t emit_AND_imm(uint16_t dst_register, uint16_t src_register, uint8_t value)
{
    check_register_index(dst_register);
    check_register_index(src_register);

    uint16_t instruction = AND << 12;
    instruction |= (dst_register << 9);
    instruction |= (src_register << 6);
    instruction |= (value & 0x1f);
    return instruction;
}

uint16_t emit_AND_reg(uint16_t dst_register, uint16_t src_register, uint16_t src2_register)
{
    check_register_index(dst_register);
    check_register_index(src_register);
    check_register_index(src2_register);

    uint16_t instruction = AND << 12;
    instruction |= (dst_register << 9);
    instruction |= (src_register << 6);
    instruction |= (src2_register);
    return instruction;
}

uint16_t emit_LD(int16_t pc_offset, uint16_t dst_register)
{
    check_register_index(dst_register);
    uint16_t instruction = LD << 12;
    instruction |= pc_offset & 0x1ff;
    instruction |= (dst_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_ST(uint16_t pc_offset, uint16_t src_register)
{
    check_register_index(src_register);
    uint16_t instruction = ST << 12;
    instruction |= pc_offset & 0x1ff;
    instruction |= (src_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_LDI(int16_t pc_offset, uint16_t dst_register)
{
    check_register_index(dst_register);
    uint16_t instruction = LDI << 12;
    instruction |= pc_offset & 0x1ff;
    instruction |= (dst_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_STI(uint16_t pc_offset, uint16_t src_register)
{
    check_register_index(src_register);
    uint16_t instruction = STI << 12;
    instruction |= pc_offset & 0x1ff;
    instruction |= (src_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_LDR(int16_t pc_offset, uint16_t dst_register, uint16_t base_register)
{
    check_register_index(dst_register);
    check_register_index(base_register);
    uint16_t instruction = LDR << 12;
    instruction |= pc_offset & 0x3f;
    instruction |= (dst_register & 0x7) << 9;
    instruction |= (base_register & 0x7) << 6;
    return instruction;
}

uint16_t emit_STR(uint16_t pc_offset, uint16_t src_register)
{
    check_register_index(src_register);
    uint16_t instruction = STR << 12;
    instruction |= pc_offset & 0x1f;
    instruction |= (src_register & 0x7) << 9;
    return instruction;
}

uint16_t emit_LEA(int16_t pc_offset, uint16_t dst_register)
{
    check_register_index(dst_register);
    uint16_t instruction = LEA << 12;
    instruction |= pc_offset & 0x1ff;
    return instruction;
}

uint16_t emit_TRAP(uint8_t trap_code)
{
    if (trap_code != 0x21 && trap_code != 0x23 && trap_code != 0x25)
        fatalf("Invalid trap code: %#02x\n", trap_code);
    uint16_t instruction = 0xf000 + trap_code;
    return instruction;
}

uint16_t emit_BR(int16_t pc_offset, bool positive, bool zero, bool negative)
{
    uint16_t instruction = BR << 12;
    if (positive)
        instruction |= (1 << 9);
    if (zero)
        instruction |= (1 << 10);
    if (negative)
        instruction |= (1 << 11);
    return instruction;
}

uint16_t emit_JMP(uint16_t src_register)
{
    check_register_index(src_register);
    uint16_t instruction = JMP << 12;
    instruction |= (src_register & 0x1f) << 6;
    return instruction;
}
