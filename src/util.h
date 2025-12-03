#pragma once

void fatalf(const char* format, ...) __attribute__((noreturn));
char* file_read_text(const char* filename);
