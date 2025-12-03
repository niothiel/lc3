#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "emulator.h"
#include "opcode.h"

void print_usage(char* first_arg)
{
    fprintf(stderr, "Usage: %s <command> [<args>]\n", first_arg);
    fprintf(stderr, "\n");
    fprintf(stderr, "Subcommands:\n");
    fprintf(stderr, "   exec <file>.bin : Execute machine code.\n");
    fprintf(stderr, "   asm <file>.s    : Assemble a file into machine code.\n");
    fprintf(stderr, "   run <file>.s    : Assemble a file and execute it.\n");
    exit(EXIT_FAILURE);
}

static char* replace_ext(char* filename, char* new_ext)
{
    size_t len = strlen(filename);
    bool found = false;
    char* end = filename + len;
    for (; end > filename; end--) {
        if (*end == '.') {
            found = true;
            break;
        }
    }

    if (!found)
        return strdup(filename);

    size_t len_no_ext = end - filename;
    size_t new_len = len_no_ext + strlen(new_ext) + 1;
    char* new_filename = (char*)malloc(new_len);

    strncpy(new_filename, filename, len_no_ext);
    new_filename[len_no_ext] = '\0';

    strlcat(new_filename, new_ext, strlen(new_ext));
    return new_filename;
}

static void exec_file(char* filename)
{
    uint16_t* memory = assembler_read_bin_file(filename);
    lc3_state_t state;
    lc3_state_init(&state);

    memcpy(state.mem, memory, 65536 * sizeof(*memory));
    free(memory);

    lc3_state_step_until_halt(&state);
}

static void assemble_file(char* filename)
{
    uint16_t* memory = assembler_assemble_file(filename);
    char* new_filename = replace_ext(filename, "bin");
    assembler_write_bin_file(memory, new_filename);
    free(memory);
    printf("Wrote assembled machine code to: %s\n", new_filename);
}

static void run_file(char* filename)
{
    uint16_t* memory = assembler_assemble_file(filename);
    lc3_state_t state;
    lc3_state_init(&state);

    memcpy(state.mem, memory, 65536 * sizeof(*memory));
    free(memory);

    lc3_state_step_until_halt(&state);
}

int main(int argc, char* argv[])
{
    // test_suite();
    // exit(0);

    if (argc != 3)
        print_usage(argv[0]);

    char* subcommand = argv[1];
    if (strcmp(subcommand, "exec") == 0) {
        exec_file(argv[2]);
    } else if (strcmp(subcommand, "asm") == 0) {
        assemble_file(argv[2]);
    } else if (strcmp(subcommand, "run") == 0) {
        run_file(argv[2]);
    } else {
        fprintf(stderr, "fatal: unknown subcommand: %s\n", subcommand);
        print_usage(argv[0]);
    }
}
