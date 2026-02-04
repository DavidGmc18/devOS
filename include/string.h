#pragma once

#include <stddef.h>
#include <stdbool.h>

size_t strlen(const char* str);

char* strncpy(char *dst, const char *src, size_t count);

const char* strchr(const char* str, char chr);

char* strcpy(char* dst, const char* src);

bool islower(char chr);

char toupper(char chr);