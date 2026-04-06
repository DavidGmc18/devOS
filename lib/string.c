#include "string.h"
#include <stdint.h>

#define FILL_MULTIPLIER ((uintptr_t)(-1ULL / 0xFF))

void* memset(void* ptr, int value, size_t num) {
    if (!ptr) return NULL;

    uint8_t* byte_ptr = ptr;
    for (size_t i = 0; i < num; i++)
        byte_ptr[i] = value;

    return ptr;
}

size_t strlen(const char *s) {
    if (!s) return 0;
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

size_t wcslen(const wchar_t *s) {
    if (!s) return 0;
    const wchar_t *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

void* memcpy(void* dest, const void* src, size_t count) {
    if (!dest) return NULL;
    if (!src) return NULL;
    
    uint8_t* byte_dest = dest;
    const uint8_t* byte_src = src;
    for (size_t i = 0; i < count; i++) {
        byte_dest[i] = byte_src[i];
    }

    return dest;
}