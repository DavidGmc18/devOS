#include "page.h"
#include <stdbool.h>
#include <stddef.h>

#include <arch/i686/printk.h>

#define KERNEL_END 0x200000
#define PAGE_MAP_PAGES 1

static uint8_t* PAGE_MAP = (uint8_t*)KERNEL_END;
static uintptr_t FREE_MEMORY_START = KERNEL_END + PAGE_MAP_PAGES * PAGE_SIZE;

#define INVALID_BLOCK 0

static inline void set_page(uintptr_t page) {
    PAGE_MAP[page/8] |= (1 << (page % 8));
}

static inline void release_page(uintptr_t page) {
    PAGE_MAP[page/8] &= ~(1 << (page % 8));
}

static inline bool read_page(uintptr_t page) {
    return (PAGE_MAP[page/8] >> (page % 8)) & 1;
}

static uintptr_t find_block(uintptr_t count) {
    uintptr_t page = FREE_MEMORY_START / PAGE_SIZE;
    uintptr_t max_page = PAGE_MAP_PAGES * PAGE_SIZE * 8; // TODO adjust base don E820

    uintptr_t consecutive = 0;
    uintptr_t start_page = page;

    while (page < max_page) {
        if (read_page(page)) {
            consecutive = 0;
            start_page = page + 1;
        } else {
            consecutive++;
            if (consecutive >= count)
                break;
        }

        page++;
    }

    if (consecutive < count) {
        return INVALID_BLOCK;
    }

    return start_page;
}

PageBlock alloc_page(uintptr_t count) {
    uintptr_t page = find_block(count);

    if (page == INVALID_BLOCK)
        return (PageBlock){NULL, 0};

    uintptr_t remaining = count;
    uintptr_t current_page = page;
    while (remaining > 0) {
        set_page(current_page);
        remaining--;
        current_page++;
    }

    return (PageBlock){(void*)(page*PAGE_SIZE), count};
}

void free_page(PageBlock* block) {
    uintptr_t page = (uintptr_t)block->ptr / PAGE_SIZE;

    uintptr_t remaining = block->count;
    uintptr_t current_page = page;
    while (remaining > 0) {
        release_page(page);
        remaining--;
        page++;
    }

    // Invalidate PageBlock
    uintptr_t* raw_block = (uintptr_t*)block;
    raw_block[0] = (uintptr_t)NULL;
    raw_block[1] = 0;
}