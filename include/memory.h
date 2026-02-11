#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);

#ifdef __cplusplus
}
#endif