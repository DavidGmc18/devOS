#pragma once

#define PAGE_SIZE (4096)

#define PG_RESERVED (0U)
#define PG_BUDDY (1U) // Mark head of free buddy block
#define PG_ALLOCATED (2U) // Mark head of allocation / page that can be used to free block
#define PG_TAIL (3U) // Mark non-head pages from any block (buddy, allocated)

#define PAGE_MAX_ORDER 10

struct page {
    unsigned char state;
    unsigned char order;
    struct page* prev;
    struct page* next;
};