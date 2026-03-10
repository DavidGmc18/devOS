#include "string.h"

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