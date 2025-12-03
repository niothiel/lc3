#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatalf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    va_end(args);

    // Fatal means exit.
    exit(EXIT_FAILURE);
}

char* file_read_text(const char* filename)
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
