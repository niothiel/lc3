#pragma once

typedef enum {
    // Operations
    NOT = 0x9, // 1001
    ADD = 0x1, // 0001
    AND = 0x5, // 0101

    // Data Movement
    LD = 0x2, // 0010
    ST = 0x3, // 0011
    LDI = 0xa, // 1010
    STI = 0xb, // 1011
    LDR = 0x6, // 0110
    STR = 0x7, // 0111
    LEA = 0xe, // 1110

    // Control
    BR = 0x0, // 0000
    JMP = 0xc, // 1100
    TRAP = 0xf, // 1111
} opcode;
