#include "emulator.h"
#include <stdio.h>
#include <stdlib.h>

static void assert_mem(lc3_state_t* state, uint16_t mem_addr, uint16_t expected_value)
{
    uint16_t actual_value = state->mem[mem_addr];
    if (actual_value != expected_value) {
        fprintf(stderr, "Unexpected memory value at address %#04x: %#04x. Expected: %#04x\n", mem_addr, actual_value, expected_value);
        exit(1);
    }
}

static void assert_register(lc3_state_t* state, uint16_t register_index, uint16_t expected_value)
{
    uint16_t actual_value = state->gp_registers[register_index];
    if (actual_value != expected_value) {
        fprintf(stderr, "Unexpected value in register R%d: %#04x. Expected: %#04x\n", register_index, actual_value, expected_value);
        exit(1);
    }
}

static void assert_pc(lc3_state_t* state, uint16_t expected_pc)
{
    if (state->pc != expected_pc) {
        fprintf(stderr, "Unexpected PC Value: %#04x. Expected: %#04x\n", state->pc, expected_pc);
        exit(1);
    }
}

void test_suite(void)
{
    lc3_state_t state;
    lc3_state_init(&state);

    // Test NOT
    lc3_state_init(&state);
    state.mem[0x3000] = 0x903f; // NOT R0, R0
    state.gp_registers[0] = 0x00ff;
    lc3_state_step(&state);
    assert_register(&state, 0, 0xff00);

    // Test ADD (Immediate, Negative)
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1030; // ADD R0, R0, -16
    lc3_state_step(&state);
    if ((int16_t)state.gp_registers[0] != -16) {
        fprintf(stderr, "Invalid value received. Expected: -15, was: %d\n", (int16_t)state.gp_registers[0]);
        exit(1);
    }

    // Test ADD (Immediate, Positive)
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1021; // ADD R0, R0, 1
    lc3_state_step(&state);
    assert_register(&state, 0, 1);

    // Test ADD (Register)
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1021; // ADD R0, R0, 1
    state.mem[0x3001] = 0x1262; // ADD R1, R1, 2
    state.mem[0x3002] = 0x1401; // ADD R2, R0, R1
    lc3_state_step(&state);
    lc3_state_step(&state);
    lc3_state_step(&state);
    assert_register(&state, 2, 3);

    // Test ST
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1025; // ADD R0, R0, 5
    state.mem[0x3001] = 0x3000; // ST R0, 0
    state.mem[0x3002] = 0x9999; // [invalid placeholder]
    lc3_state_step(&state);
    lc3_state_step(&state);
    assert_mem(&state, 0x3002, 0x5);

    // Test STR
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1025; // ADD R0, R0, 5
    state.mem[0x3001] = 0x7001; // STR R0, R0, 1
    lc3_state_step(&state);
    lc3_state_step(&state);
    assert_mem(&state, 0x0006, 0x5);

    // Test LD
    lc3_state_init(&state);
    state.mem[0x3000] = 0x2000; // LD R0, 0
    state.mem[0x3001] = 0x9999; // [value to load into R0]
    lc3_state_step(&state);
    assert_register(&state, 0, 0x9999);

    // Test LDR
    lc3_state_init(&state);
    state.mem[0x3000] = 0x6005; // LDR R0, R0, 5
    state.mem[0x5] = 0x9999; // [value to load into R0]
    lc3_state_step(&state);
    assert_register(&state, 0, 0x9999);

    // Test JMP
    lc3_state_init(&state);
    state.mem[0x3000] = 0x1025; // ADD R0, R0, 5
    state.mem[0x3001] = 0xc000; // JMP R0
    lc3_state_step(&state);
    lc3_state_step(&state);
    assert_pc(&state, 0x5);

    // Test LEA
    lc3_state_init(&state);
    state.mem[0x3000] = 0xe201; // LEA R1, 1
    lc3_state_step(&state);
    assert_register(&state, 1, 0x3002);

    // Test BR (branched on zero)
    lc3_state_init(&state);
    state.mem[0x3000] = 0x0406; // BRZ 6
    lc3_state_step(&state);
    assert_pc(&state, 0x3007);

    // Test BR (did not branch on zero)
    lc3_state_init(&state);
    state.mem[0x3000] = 0x0406; // BRZ 6
    state.cond = COND_NEG;
    lc3_state_step(&state);
    assert_pc(&state, 0x3001);
}
