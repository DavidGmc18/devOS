#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <hal/vfs.h>

void clrscr();

void fputc(char ch, fd_t stream);
void fputs(const char* str, fd_t stream);

void vfprintf(fd_t stream, const char* format, va_list args);
void fprintf(fd_t stream, const char* format, ...);

void putc(char ch);
void puts(const char* str);

void printf(const char* format, ...);