#include "assembler.h"
#include "emit.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SIZE 65536

typedef struct {
    uint16_t* program;
    uint16_t pc;
} program_state_t;

typedef enum {
    REGISTER,
    SCALAR,
    COMMAND,
    END,
} token_type;

typedef union {
    uint16_t value;
    char* command;
} value_u;

typedef struct {
    token_type type;
    value_u value;
} token_t;

typedef struct {
    char* line;
    char* offset;
} lexer_t;

static char* skip_whitespace(char* string)
{
    if (string == NULL)
        return NULL;

    while (*string == ' ' || *string == '\t')
        string++;
    return string;
}

static bool is_stopchar(char c)
{
    return c == ' ' || c == '\r' || c == '\n' || c == ',' || c == '\0';
}

char* lexer_next_str(lexer_t* lexer)
{
    if (lexer->line == NULL)
        fatalf("Line cannot be NULL");
    if (lexer->offset == NULL)
        fatalf("Offset cannot be NULL");

    lexer->offset = skip_whitespace(lexer->offset);
    char* end = lexer->offset;
    while (!is_stopchar(end[0]))
        end++;

    size_t size = end - lexer->offset;
    char* ret = strndup(lexer->offset, size);
    lexer->offset = end + 1;
    return ret;
}

token_t lexer_next_token(lexer_t* lexer)
{
    char* str = lexer_next_str(lexer);
    token_t token = { 0 };
    if (strlen(str) == 0) {
        token.type = END;
        free(str);
        return token;
    } else if (str[0] == 'R') {
        token.type = REGISTER;
        token.value.value = atoi(str + 1);
        free(str);
        return token;
    } else if (str[0] == '#') {
        token.type = SCALAR;
        token.value.value = atoi(str + 1);
        free(str);
        return token;
    } else {
        token.type = COMMAND;
        token.value.command = str;
        return token;
    }
}

static void assert_register_token(lexer_t* lexer, token_t token)
{
    if (token.type != REGISTER)
        fatalf("Expected register, but was: %d in line: %s\n", token.type, lexer->line);
}

static void assert_scalar_token(lexer_t* lexer, token_t token)
{
    if (token.type != SCALAR)
        fatalf("Expected scalar, but was: %d in line: %s\n", token.type, lexer->line);
}

static void process_NOT(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    program->program[program->pc] = emit_NOT(dst_token.value.value, src_token.value.value);
    program->pc++;
}

static void process_ADD(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t src2_token = lexer_next_token(lexer);
    if (src2_token.type != REGISTER && src2_token.type != SCALAR)
        fatalf("Expected scalar or register token, but got: %d\n", src2_token.type);

    if (src2_token.type == SCALAR) {
        program->program[program->pc] = emit_ADD_imm(dst_token.value.value, src_token.value.value, src2_token.value.value);
    } else {
        program->program[program->pc] = emit_ADD_reg(dst_token.value.value, src_token.value.value, src2_token.value.value);
    }
    program->pc++;
}

static void process_AND(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t src2_token = lexer_next_token(lexer);
    if (src2_token.type != REGISTER && src2_token.type != SCALAR)
        fatalf("Expected scalar or register token, but got: %d\n", src2_token.type);

    if (src2_token.type == SCALAR) {
        program->program[program->pc] = emit_AND_imm(dst_token.value.value, src_token.value.value, src2_token.value.value);
    } else {
        program->program[program->pc] = emit_AND_reg(dst_token.value.value, src_token.value.value, src2_token.value.value);
    }
    program->pc++;
}

static void process_LD(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_LD(pc_offset_token.value.value, dst_token.value.value);
    program->pc++;
}

static void process_ST(program_state_t* program, lexer_t* lexer)
{
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, src_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_ST(pc_offset_token.value.value, src_token.value.value);
    program->pc++;
}

static void process_LDI(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_LDI(pc_offset_token.value.value, dst_token.value.value);
    program->pc++;
}

static void process_STI(program_state_t* program, lexer_t* lexer)
{
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, src_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_STI(pc_offset_token.value.value, src_token.value.value);
    program->pc++;
}

static void process_LDR(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t base_token = lexer_next_token(lexer);
    assert_register_token(lexer, base_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_LDR(pc_offset_token.value.value, dst_token.value.value, base_token.value.value);
    program->pc++;
}

static void process_STR(program_state_t* program, lexer_t* lexer)
{
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, src_token);
    token_t base_token = lexer_next_token(lexer);
    assert_register_token(lexer, base_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_STR(pc_offset_token.value.value, src_token.value.value, base_token.value.value);
    program->pc++;
}

static void process_LEA(program_state_t* program, lexer_t* lexer)
{
    token_t dst_token = lexer_next_token(lexer);
    assert_register_token(lexer, dst_token);
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    program->program[program->pc] = emit_LEA(pc_offset_token.value.value, dst_token.value.value);
    program->pc++;
}

static void process_TRAP(program_state_t* program, lexer_t* lexer)
{
    token_t trap_code_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, trap_code_token);

    program->program[program->pc] = emit_TRAP(trap_code_token.value.value);
    program->pc++;
}

static void process_BR(program_state_t* program, lexer_t* lexer, char* command)
{
    token_t pc_offset_token = lexer_next_token(lexer);
    assert_scalar_token(lexer, pc_offset_token);

    bool positive = false;
    bool zero = false;
    bool negative = false;

    char* flags = command + 2;
    char c = flags[0];
    while (c != '\0') {
        switch (c) {
        case 'p':
            positive = true;
            break;
        case 'n':
            negative = true;
            break;
        case 'z':
            zero = true;
            break;
        default:
            printf("Unknown character in branch instruction: %c", c);
        }
        flags++;
        c = flags[0];
    }

    program->program[program->pc] = emit_BR(pc_offset_token.value.value, positive, zero, negative);
    program->pc++;
}

static void process_JMP(program_state_t* program, lexer_t* lexer)
{
    token_t src_token = lexer_next_token(lexer);
    assert_register_token(lexer, src_token);

    program->program[program->pc] = emit_JMP(src_token.value.value);
    program->pc++;
}

static void process_HALT(program_state_t* program, lexer_t* lexer)
{
    (void)lexer;
    program->program[program->pc] = emit_TRAP(0x25);
}

static void process_line(program_state_t* program, char* line)
{
    // Skip comments.
    if (line[0] == ';')
        return;

    lexer_t lexer = {
        .line = line,
        .offset = line,
    };

    printf("Processing Line: %s\n", line);
    token_t command_token = lexer_next_token(&lexer);
    if (command_token.type != COMMAND)
        fatalf("Expected command, but was: %s", line);
    char* command = command_token.value.command;

    if (strcmp(command, "NOT") == 0) {
        process_NOT(program, &lexer);
    } else if (strcmp(command, "ADD") == 0) {
        process_ADD(program, &lexer);
    } else if (strcmp(command, "AND") == 0) {
        process_AND(program, &lexer);
    } else if (strcmp(command, "LD") == 0) {
        process_LD(program, &lexer);
    } else if (strcmp(command, "ST") == 0) {
        process_ST(program, &lexer);
    } else if (strcmp(command, "LDI") == 0) {
        process_LDI(program, &lexer);
    } else if (strcmp(command, "STI") == 0) {
        process_STI(program, &lexer);
    } else if (strcmp(command, "LDR") == 0) {
        process_LDR(program, &lexer);
    } else if (strcmp(command, "STR") == 0) {
        process_STR(program, &lexer);
    } else if (strcmp(command, "LEA") == 0) {
        process_LEA(program, &lexer);
    } else if (strcmp(command, "TRAP") == 0) {
        process_TRAP(program, &lexer);
    } else if (strcmp(command, "BR") == 0) {
        process_BR(program, &lexer, command);
    } else if (strcmp(command, "HALT") == 0) {
        process_HALT(program, &lexer);
    } else if (strcmp(command, "JMP") == 0) {
        process_JMP(program, &lexer);
    } else {
        printf("Unsupported instruction: %s\n", command);
    }

    // Free the allocated command string.
    free(command_token.value.command);
}

static program_state_t* program_state_new(void)
{
    program_state_t* ps = (program_state_t*)malloc(sizeof(*ps));
    ps->program = (uint16_t*)calloc(PROGRAM_SIZE, sizeof(*ps->program));
    ps->pc = 0x3000;
    return ps;
}

uint16_t* assembler_assemble_program(char* assembly)
{
    if (assembly == NULL)
        return NULL;
    program_state_t* program = program_state_new();

    char* line = strtok(assembly, "\r\n");
    while (line != NULL) {
        process_line(program, line);
        line = strtok(NULL, "\r\n");
    }

    return program->program;
}

uint16_t* assembler_assemble_file(char* filename)
{
    char* assembly = file_read_text(filename);
    return assembler_assemble_program(assembly);
}

uint16_t* assembler_read_bin_file(char* filename)
{
    FILE* f = fopen(filename, "r");
    if (f == NULL)
        return NULL;

    uint16_t* memory = (uint16_t*)calloc(65536, sizeof(*memory));
    int count_read = fread(memory, sizeof(*memory), 65536, f);
    fclose(f);
    if (count_read != 65536) {
        fprintf(stderr, "Error: Malformed binary file, expected 65536 entries, but got: %d\n", count_read);
        free(memory);
        return NULL;
    }

    return memory;
}

void assembler_write_bin_file(uint16_t* memory, char* filename)
{
    FILE* f = fopen(filename, "w");
    if (f == NULL)
        fatalf("Failed to open output file: %s", filename);

    int write_count = fwrite(memory, sizeof(*memory), 65536, f);
    if (write_count != 65536)
        fatalf("Did not write the appropriate amount of bytes. Expected: 65536. Actual: %d", write_count);

    fclose(f);
}
