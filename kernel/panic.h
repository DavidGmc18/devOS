#pragma once

#include <stdarg.h>

void __attribute__((noreturn)) vpanic(const char* format, va_list args);
void __attribute__((noreturn)) panic(const char* format, ...);