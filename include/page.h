#pragma once

#define PAGE_SIZE (4096)

#define PG_RESERVED (1 << 0)
#define PG_BUDDY (1 << 1)

#define PAGE_MAX_ORDER 10

struct page {
    unsigned char flags;
    unsigned char order;
    struct page* next_free;
};