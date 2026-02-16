#include "page.h"
#include <stdbool.h>
#include <stddef.h>
#include <memory.h>
#include <math.h>

#define KERNEL_END 0x200000
static uintptr_t KERNEL_END_PAGE;

uint8_t* PAGE_MAP = (uint8_t*)KERNEL_END;
static uintptr_t PAGE_COUNT;

static uintptr_t total_mem;
static uintptr_t usable_mem;
static uintptr_t unused_mem;

static inline void set_page(uintptr_t page) {
    PAGE_MAP[page/8] |= (1 << (page % 8));
}

static inline void release_page(uintptr_t page) {
    PAGE_MAP[page/8] &= ~(1 << (page % 8));
}

static inline bool read_page(uintptr_t page) {
    return (PAGE_MAP[page/8] >> (page % 8)) & 1;
}

static void set_pages(uintptr_t page, uintptr_t num_pages) {
    for (uintptr_t i = 0; i < num_pages; i++) {
        set_page(page + i);
    }
}

static void release_pages(uintptr_t page, uintptr_t num_pages) {
    for (uintptr_t i = 0; i < num_pages; i++) {
        release_page(page + i);
    }
}

// TODO this is illegal
static void invalidate_block(PAGE_Block* page_block) {
    memset(page_block, 0, sizeof(PAGE_Block));
    // TODO assert
}

static uintptr_t find_block(uintptr_t count) {
    uintptr_t page = KERNEL_END / PAGE_SIZE;

    uintptr_t consecutive = 0;
    uintptr_t start_page = page;

    while (page < PAGE_COUNT) {
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
        return INVALID_PAGE;
    }

    return start_page;
}

static void reserve_and_count(const MemoryInfo* mem_info, uintptr_t after_map_page) {
    total_mem = 0; // total physical memory reported by E820
    usable_mem = 0; // memory marked usable by E820
    unused_mem = 0; // memory actually free for allocation

    for (int i = 0; i < mem_info->block_count; i++) {
        uintptr_t start_page = mem_info->blocks[i].base / PAGE_SIZE;
        
        uintptr_t last_adress = mem_info->blocks[i].base + mem_info->blocks[i].length;
        uintptr_t last_page = DIV_UP(last_adress, PAGE_SIZE);
        uintptr_t end_page = MIN(last_page, PAGE_COUNT);
        
        total_mem += mem_info->blocks[i].length;
    
        if (mem_info->blocks[i].type == E820_USABLE) {
            usable_mem += mem_info->blocks[i].length;

            // unused_mem
            if (end_page > after_map_page) {
                if (start_page >= after_map_page) {
                    unused_mem += mem_info->blocks[i].length;
                } else {
                    unused_mem += (end_page - after_map_page) * PAGE_SIZE;
                }
            }
        } else {
            uintptr_t num_pages = end_page - start_page;
            set_pages(start_page, num_pages);
        }
    }
}

void PAGE_initialize(const MemoryInfo* mem_info) {
    // Physical mem
    MemoryBlock last_block = mem_info->blocks[mem_info->block_count-1];
    PAGE_COUNT = (last_block.base + last_block.length) / PAGE_SIZE;
    memset(PAGE_MAP, 0, PAGE_COUNT * PAGE_SIZE); // Mark all pages as free by default

    // Kernel space & page map
    KERNEL_END_PAGE = DIV_UP(KERNEL_END, PAGE_SIZE);
    uint16_t entries_per_page = PAGE_SIZE * 8;
    uintptr_t map_pages = DIV_UP(PAGE_COUNT, entries_per_page);
    set_pages(0, KERNEL_END_PAGE + map_pages);

    // Reserve regions
    reserve_and_count(mem_info, KERNEL_END_PAGE + map_pages);
}

PAGE_Block PAGE_alloc(uintptr_t num_pages) {
    uintptr_t page = find_block(num_pages);

    if (page == INVALID_PAGE)
        return (PAGE_Block){NULL, 0};

    set_pages(page, num_pages);
    return (PAGE_Block){(void*)(page*PAGE_SIZE), num_pages};
}

void PAGE_free(PAGE_Block* page_block) {
    release_pages((uintptr_t)page_block->address / PAGE_SIZE, page_block->page_count);
    invalidate_block(page_block);
}

uintptr_t PAGE_total_mem() {
    return total_mem;
}

uintptr_t PAGE_usable_mem() {
    return usable_mem;
}

uintptr_t PAGE_unused_mem() {
    return unused_mem;
}