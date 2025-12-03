// LC-3 Emulator
// Based off of: https://www.cs.utexas.edu/~fussell/courses/cs310h/lectures/Lecture_10-310h.pdf
// Better Explainer: https://medium.com/@saehwanpark/diving-deeper-into-lc-3-from-opcodes-to-machine-code-4637cf00c878
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulator.h"
#include "opcode.h"

static int16_t sign_extend(uint16_t raw, int n_bits)
{
    int lower_mask = (1 << (n_bits)) - 1;
    int sign_mask = 1 << (n_bits - 1);
    int raw_value = raw & lower_mask;
    int sign = 1 - ((uint16_t)(raw & sign_mask) >> (n_bits - 2));
    int16_t value = sign * raw_value;
    return value;
}

void lc3_state_init(lc3_state_t* state)
{
    if (state == NULL)
        return;
    memset(state->mem, 0, MEMORY_MAX * sizeof(*state->mem));
    for (int i = 0; i < 8; i++)
        state->gp_registers[i] = 0;
    state->pc = 0x3000;
    state->cond = COND_ZERO;
    state->halted = 0;
}

static void lc3_incr_pc(lc3_state_t* state)
{
    if (state->pc == MEMORY_MAX - 1) {
        fprintf(stderr, "Attempted to go past PC register.");
        exit(1);
    }
    if (state->halted) {
        fprintf(stderr, "CPU is halted. Cannot continue.");
        exit(1);
    }
    state->pc++;
}

static void handle_BR(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int pc_offset = sign_extend(instruction, 9);
    bool br_negative = (bool)(instruction >> 11 & 0x1);
    bool br_zero = (bool)(instruction >> 10 & 0x1);
    bool br_positive = (bool)(instruction >> 9 & 0x1);

    bool should_branch = false;
    if (br_negative && state->cond == COND_NEG)
        should_branch = true;
    if (br_zero && state->cond == COND_ZERO)
        should_branch = true;
    if (br_positive && state->cond == COND_POS)
        should_branch = true;

    if (should_branch)
        state->pc += pc_offset;
}

static void handle_LEA(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int pc_offset = sign_extend(instruction, 9);
    uint16_t dst_register_idx = instruction >> 9 & 0x7;
    state->gp_registers[dst_register_idx] = state->pc + pc_offset;
}

static void handle_LD(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int pc_offset = sign_extend(instruction, 9);
    uint16_t dst_register_idx = instruction >> 9 & 0x7;
    uint16_t memory_value = state->mem[state->pc + pc_offset];
    state->gp_registers[dst_register_idx] = memory_value;
}

static void handle_LDR(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int offset = sign_extend(instruction, 6);
    uint16_t dst_register_idx = instruction >> 9 & 0x7;
    uint16_t base_register_idx = instruction >> 6 & 0x7;

    uint16_t memory_location = state->gp_registers[base_register_idx] + offset;
    uint16_t memory_value = state->mem[memory_location];
    state->gp_registers[dst_register_idx] = memory_value;
}

static void handle_ST(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int pc_offset = sign_extend(instruction, 9);
    uint16_t src_register_idx = instruction >> 9 & 0x7;
    uint16_t value = state->gp_registers[src_register_idx];
    state->mem[state->pc + pc_offset] = value;
}

static void handle_STR(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    int offset = sign_extend(instruction, 5);
    uint16_t src_register_idx = instruction >> 9 & 0x7;
    uint16_t value = state->gp_registers[src_register_idx];

    uint16_t base_register_idx = instruction >> 6 & 0x7;
    uint16_t memory_location = state->gp_registers[base_register_idx] + offset;
    state->mem[memory_location] = value;
}

static void handle_ADD(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    uint16_t src_register_idx = instruction >> 6 & 0x7;
    uint16_t src_value = state->gp_registers[src_register_idx];
    uint16_t immediate_bit = instruction >> 5 & 0x1;
    int src2_value = 0;
    if (immediate_bit) {
        src2_value = sign_extend(instruction, 5);
    } else {
        uint16_t src2_register_idx = instruction & 0x7;
        src2_value = state->gp_registers[src2_register_idx];
    }

    uint16_t dst_register_idx = instruction >> 9 & 0x7;
    state->gp_registers[dst_register_idx] = src_value + src2_value;
}

static void handle_NOT(lc3_state_t* state, uint16_t instruction)
{
    lc3_incr_pc(state);
    uint16_t src_register_idx = instruction >> 6 & 0x7;
    uint16_t src_value = state->gp_registers[src_register_idx];

    uint16_t dst_register_idx = instruction >> 9 & 0x7;
    uint16_t dst_value = ~src_value;
    state->gp_registers[dst_register_idx] = dst_value;
}

static void handle_JMP(lc3_state_t* state, uint16_t instruction)
{
    uint16_t register_idx = instruction >> 6 & 0x7;
    state->pc = state->gp_registers[register_idx];
}

static void handle_TRAP(lc3_state_t* state, uint16_t instruction)
{
    (void)state;
    int chr = 0;

    uint16_t trap_code = instruction & 0xff;
    switch (trap_code) {
    case 0x25:
        state->halted = true;
        break;
    case 0x21:
        chr = state->gp_registers[0];

        printf("OUTPUT: %c (intval: %d)\n", chr, chr);
        state->pc++;
        break;
    case 0x23:
        chr = 500;
        while (chr > 255) {
            printf("INPUT: ");
            chr = getchar();
        }
        printf("Got character: %d\n", chr);
        state->gp_registers[0] = (uint16_t)chr;
        state->pc++;
        break;
    default:
        fprintf(stderr, "Trap code not implemented/invalid: %#2x\n", trap_code);
        exit(1);
    }
}

void lc3_state_step(lc3_state_t* state)
{
    if (state == NULL)
        return;
    if (state->halted) {
        fprintf(stderr, "CPU is halted. Cannot continue.");
        exit(1);
    }

    uint16_t instruction = state->mem[state->pc];
    printf("PC[%#04x] = %#04x\n", state->pc, instruction);
    uint16_t opcode = instruction >> 12;
    switch (opcode) {
    case NOT:
        handle_NOT(state, instruction);
        break;
    case LD:
        handle_LD(state, instruction);
        break;
    case LDR:
        handle_LDR(state, instruction);
        break;
    case ST:
        handle_ST(state, instruction);
        break;
    case STR:
        handle_STR(state, instruction);
        break;
    case LEA:
        handle_LEA(state, instruction);
        break;
    case ADD:
        handle_ADD(state, instruction);
        break;
    case JMP:
        handle_JMP(state, instruction);
        break;
    case TRAP:
        handle_TRAP(state, instruction);
        break;
    case BR:
        handle_BR(state, instruction);
        break;
    default:
        printf("Opcode not supported: %#2x\n", opcode);
        exit(1);
    }
}

void lc3_state_step_until_halt(lc3_state_t* state)
{
    while (!state->halted)
        lc3_state_step(state);
}
