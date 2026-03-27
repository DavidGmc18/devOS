#include "string.h"
#include <stdint.h>

#define FILL_MULTIPLIER ((uintptr_t)(-1ULL / 0xFF))

void* memset(void* ptr, int value, size_t num) {
    uintptr_t fill = (uint8_t)value * FILL_MULTIPLIER;
    size_t words = num / sizeof(uintptr_t);

    uintptr_t* native_ptr = ptr;
    for (size_t i = 0; i < words; i++)
        native_ptr[i] = fill;

    uint8_t* byte_ptr = ptr;
    for (size_t i = words * sizeof(uintptr_t); i < num; i++)
        byte_ptr[i] = value;

    return ptr;
}