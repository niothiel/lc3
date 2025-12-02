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

    char c = string[0];
    while (c == ' ' || c == '\t')
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

void assert_register_token(lexer_t* lexer, token_t token)
{
    if (token.type != REGISTER)
        fatalf("Expected register, but was: %d in line: %s\n", token.type, lexer->line);
}

void process_ADD(program_state_t* program, lexer_t* lexer)
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

void process_HALT(program_state_t* program, lexer_t* lexer)
{
    (void)lexer;
    program->program[program->pc] = emit_TRAP(0x25);
}

void process_line(program_state_t* program, char* line)
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

    if (strcmp(command, "ADD") == 0) {
        process_ADD(program, &lexer);
    } else if (strcmp(command, "HALT") == 0) {
        printf("Got a HALT!\n");
    } else {
        printf("Unsupported instruction: %s\n", command);
    }
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

static char* file_read_text(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file for reading: %s\n", filename);
        return NULL;
    }

    // Seek to the end to get the file size.
    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    // Return back to the original for reading.
    fseek(f, 0, SEEK_SET);

    // Allocate an extra byte for null-termination.
    char* contents = (char*)malloc(size + 1);
    if (contents == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s", filename);
        fclose(f);
        return NULL;
    }

    fread(contents, 1, size, f);
    fclose(f);

    // Null-terminate the contents.
    contents[size] = 0;
    return contents;
}

uint16_t* assembler_assemble_file(char* filename)
{
    char* assembly = file_read_text(filename);
    return assembler_assemble_program(assembly);
}
