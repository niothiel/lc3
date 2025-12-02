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
