#pragma once

#include <stdint.h>

#define PAGE_SIZE (4096)

#define PAGE_RESERVED (1 << 0)
#define PAGE_BUDDY (1 << 1)

struct page {
    unsigned char flags;
    uint8_t order;
    struct page* next_free;
};