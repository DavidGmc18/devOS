#pragma once

#include <stddef.h>

void* memset(void* ptr, int value, size_t num);

size_t strlen(const char *s);
size_t wcslen(const wchar_t *s);

void* memcpy(void* dest, const void* src, size_t count);