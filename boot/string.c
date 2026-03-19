#include "string.h"
#include <stddef.h>

void* memset(void* ptr, uint8_t value, uint32_t size) {
    uint32_t* p32 = (uint32_t*)ptr;
    uint32_t v32 = (uint8_t)value * 0x01010101U;
    while (size >= 4) {
        *p32++ = v32; size -=4;
    }
    uint8_t* p8 = (uint8_t*)p32;
    while (size--) {
        *p8++ = (uint8_t)value;
    }
    return ptr;
}

int memcmp(const void* s1, const void* s2, uint32_t n) {
    uint8_t* u8s1 = (uint8_t*)s1;
    uint8_t* u8s2 = (uint8_t*)s2;

    for (uint32_t i = 0; i < n; i++) {
        int ret;
        if (ret = u8s1[i] - u8s2[i])
            return ret;
    }

    return 0;
}

int toupper(char ch) {
    return (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
}

char* strchr(const char* str, char ch) {
    while (*str) {
        if (*str == ch)
            return (char*)str;
        str++;
    }
    return NULL;
}