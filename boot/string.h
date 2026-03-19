#pragma once

#include <stdint.h>

void* memset(void* ptr, uint8_t value, uint32_t size);

int memcmp(const void* s1, const void* s2, uint32_t n);

int toupper(char ch);

char* strchr(const char* str, char ch);