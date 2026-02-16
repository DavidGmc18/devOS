#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void e9_putc(char ch);
void e9_putn(const char* str, size_t size);

#ifdef __cplusplus
}
#endif