#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MEMORY_MAX 65536
#define COND_NEG 0xff
#define COND_ZERO 0x00
#define COND_POS 0x1

typedef struct {
    uint16_t mem[MEMORY_MAX];
    uint16_t gp_registers[8];
    uint16_t pc;
    uint8_t cond;
    bool halted;
} lc3_state_t;

void lc3_state_init(lc3_state_t* state);
void lc3_state_step(lc3_state_t* state);
void lc3_state_step_until_halt(lc3_state_t* state);
