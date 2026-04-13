#include "mem.h"
#include <mm.h>
#include <arch/x86/e820.h>
#include <printk.h>
#include <math.h>
#include <arch/x86/bootmem.h>
#include <panic.h>

extern char KERNEL_PHYS[];

static struct page* free_matrix[PAGE_MAX_ORDER + 1];

static size_t managed_mem;

static inline void insert_free(unsigned char order, struct page* page) {
    if (order > PAGE_MAX_ORDER) panic("Order (%d) is more than or equal as limit (%d)\n", order);
    page->next_free = free_matrix[order];
    free_matrix[order] = page;
}

static inline int alignment_order(unsigned long long addr) {
    int alignment_order = (addr == 0) ? (8 * sizeof(unsigned long long)) : __builtin_ctzll(addr);
    if (alignment_order > PAGE_MAX_ORDER) return PAGE_MAX_ORDER;
    return alignment_order;
}

// Rounded down
static inline int length_order(unsigned long long length) {
    if (length == 0) return -1;
    return (8 * sizeof(unsigned long long) - 1) - __builtin_clzll(length);
}

static size_t set_buddy_range(uintptr_t start_page, uintptr_t end_page) {
    if (end_page > mem_nr_pages) end_page = mem_nr_pages;
    if (start_page >= end_page) return 0;
    for (uintptr_t j = start_page; j < end_page; j++) {
        mem_map[j].flags = PG_BUDDY;
    }
    return end_page - start_page;
}

int buddy_init() {
    struct e820_table* table = e820_get_table();
    struct e820_entry* entries = table->entries;
    uint32_t entries_count = table->entries_count;

    bootmem_lock();

    uintptr_t reserved_start_page = (uintptr_t)KERNEL_PHYS / PAGE_SIZE;
    uintptr_t reserved_end_page = DIV_UP(bootmem_get_pool_end(), PAGE_SIZE);

    size_t expected_pages = 0;
    for (uint32_t i = 0; i < entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;

        uintptr_t start_page = DIV_UP(entries[i].addr, PAGE_SIZE);
        uintptr_t end_page = (entries[i].addr + entries[i].size) / PAGE_SIZE;

        // If start/end overlaps reserved segment
        if (start_page >= reserved_start_page && start_page < reserved_end_page) start_page = reserved_end_page;
        if (end_page > reserved_start_page && end_page <= reserved_end_page) end_page = reserved_start_page;

        // If reserved segment is inside of current
        if (start_page < reserved_start_page && end_page > reserved_end_page) {
            expected_pages += set_buddy_range(start_page, reserved_start_page);
            expected_pages += set_buddy_range(reserved_end_page, end_page);
        } else {
            expected_pages += set_buddy_range(start_page, end_page);
        }
    }

    // Group
    size_t accounted_pages = 0;
    for (uintptr_t i = 0; i < mem_nr_pages;) {
        if (!(mem_map[i].flags & PG_BUDDY)) {
            i++;
            continue;
        }

        int ao = alignment_order(i);
        uintptr_t max_buddy_size = 1ULL << ao;
        uintptr_t end = ((i + max_buddy_size) <= mem_nr_pages) ? (i + max_buddy_size) : mem_nr_pages;

        uintptr_t j = i;
        while (j < end ) {
            if (!(mem_map[j].flags & PG_BUDDY)) break;
            j++;
        }

        int order = length_order(j - i);
        uintptr_t buddy_size = 1ULL << order;
        
        mem_map[i].order = order;
        insert_free(order, &mem_map[i]);

        for (j = (i + 1); j < (i + buddy_size); j++) {
            mem_map[j].flags &= ~PG_BUDDY;
        }

        accounted_pages += buddy_size;
        i += buddy_size;
    }

    if (expected_pages != accounted_pages) panic("Buddy grouping failed, expecyed %zu pages but accounted %zu\n", expected_pages, accounted_pages);
    managed_mem = accounted_pages * PAGE_SIZE;

    printk("[OK] Buddy allocator initialized, managed mem is %zu KiB\n", managed_mem / 1024);
    return 0;
}

struct page* alloc_pages(unsigned long order) {

}