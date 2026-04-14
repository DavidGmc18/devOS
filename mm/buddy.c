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

static inline void insert_free(struct page* page) {
    unsigned char order = page->order;
    if (order > PAGE_MAX_ORDER) panic("Order %d is more than or equal as limit (%d)\n", order);

    page->next = free_matrix[order];
    page->prev = NULL;

    if (page->next) page->next->prev = page;
    free_matrix[order] = page;

    page->state = PG_BUDDY;
}

static inline struct page* pop_free(unsigned char order) {
    if (order > PAGE_MAX_ORDER) panic("Order %d is more than or equal as limit (%d)\n", order);

    struct page* page = free_matrix[order];
    if (!page) return NULL;

    free_matrix[order] = page->next;
    if (free_matrix[order]) free_matrix[order]->prev = NULL;

    page->next = NULL;
    page->prev = NULL;
    return page;
}

static inline void remove_free(struct page* page) {
    unsigned char order = page->order;
    if (order > PAGE_MAX_ORDER) panic("Order %d is more than or equal as limit (%d)\n", order);

    if (page->state != PG_BUDDY) return;

    // If prev & next are NULL, it means that this page is not linked in free_matrix, or that its only entry in its order list,
    // so we check if page is not first/only entry in free_matrix[order]
    if (!page->prev && !page->next && (free_matrix[order] != page)) return; 

    if (page->prev) page->prev->next = page->next; else free_matrix[order] = page->next;
    if (page->next) page->next->prev = page->prev;

    page->next = NULL;
    page->prev = NULL;
}

static inline int alignment_order(unsigned long long pfn) {
    int alignment_order = (pfn == 0) ? (8 * sizeof(unsigned long long)) : __builtin_ctzll(pfn);
    if (alignment_order > PAGE_MAX_ORDER) return PAGE_MAX_ORDER;
    return alignment_order;
}

// Rounded down
static inline int length_order(unsigned long long length) {
    if (length == 0) return -1;
    return (8 * sizeof(unsigned long long) - 1) - __builtin_clzll(length);
}

static size_t set_first_order_buddy_for_range(uintptr_t start_page, uintptr_t end_page) {
    if (end_page > mem_nr_pages) end_page = mem_nr_pages;
    if (start_page >= end_page) return 0;
    for (uintptr_t i = start_page; i < end_page; i++) {
        mem_map[i].state = PG_BUDDY;
        mem_map[i].order = 0;
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
        if (start_page >= end_page) continue;

        // If reserved segment is inside of current
        if (start_page < reserved_start_page && end_page > reserved_end_page) {
            expected_pages += set_first_order_buddy_for_range(start_page, reserved_start_page);
            expected_pages += set_first_order_buddy_for_range(reserved_end_page, end_page);
        } else {
            expected_pages += set_first_order_buddy_for_range(start_page, end_page);
        }
    }

    // Group
    size_t accounted_pages = 0;
    for (uintptr_t i = 0; i < mem_nr_pages;) {
        if (mem_map[i].state != PG_BUDDY) {
            i++;
            continue;
        }

        size_t max_pages_in_block = 1ULL << alignment_order(i);
        uintptr_t end = ((i + max_pages_in_block) <= mem_nr_pages) ? (i + max_pages_in_block) : mem_nr_pages;

        uintptr_t j = i;
        while (j < end ) {
            if (mem_map[j].state != PG_BUDDY) break;
            j++;
        }

        uintptr_t continuous_pages_count = j - i;
        mem_map[i].order = length_order(continuous_pages_count);

        size_t pages_in_block = 1ULL << mem_map[i].order;
        for (j = (i + 1); j < (i + pages_in_block); j++) {
            mem_map[j].state = PG_TAIL;
        }

        insert_free(&mem_map[i]);
        accounted_pages += pages_in_block;
        i += pages_in_block;
    }

    if (expected_pages != accounted_pages) panic("Buddy grouping failed, expected %zu pages but accounted %zu\n", expected_pages, accounted_pages);
    managed_mem = accounted_pages * PAGE_SIZE;

    printk("[OK] Buddy allocator initialized, managed mem is %zu KiB\n", managed_mem / 1024);
    return 0;
}

struct page* alloc_pages(unsigned char order) {
    if (order > PAGE_MAX_ORDER) panic("Order %d is more than maximum of %d\n", order, PAGE_MAX_ORDER);

    struct page* page = NULL;
    for (unsigned char i = order; i <= PAGE_MAX_ORDER; i++) {
        page = pop_free(i);
        if (page) break;
    }

    if (!page) return NULL;
 
    while (page->order > order) {
        page->order -= 1;

        struct page* buddy = &page[1ULL << page->order];
        if (buddy->state != PG_TAIL) panic("Unexpected page state, expected %u(PG_TAIL) but state is %u\n", PG_TAIL, buddy->state);
        // buddy->state = PG_BUDDY; // This is reduntant
        buddy->order = page->order;
        insert_free(buddy); // sets -> buddy->state = PG_BUDDY
    }

    page->state = PG_ALLOCATED;
    return page;
}

void free_pages(struct page* page) {
    if (page->state != PG_ALLOCATED) return;

    // Set prev & next to NULL -> hinting that this page is not in free_matrix
    page->state = PG_BUDDY;
    page->prev = NULL;
    page->next = NULL;

    // Merge corrent block as much as possible and then add it to free_matrix
    while (page->order < PAGE_MAX_ORDER) {
        uintptr_t pfn = (struct page*)page - (struct page*)mem_map;
        if (alignment_order(pfn) > page->order) {
            // Potentional buddy to merge with is on rigth (after)
            struct page* buddy = &page[1ULL << page->order];
            if (buddy >= &mem_map[mem_nr_pages]) break;
            if ((buddy->state == PG_BUDDY) && (buddy->order == page->order)) {
                remove_free(buddy); // Won't be removed if its not in free_matrix
                buddy->state = PG_TAIL;
                page->order += 1;
            } else break;
        } else {
            // Potentional buddy to merge with is on left (before)
            struct page* buddy = &page[-(1ULL << page->order)];
            if (buddy < mem_map) break;
            if ((buddy->state == PG_BUDDY) && (buddy->order == page->order)) {
                remove_free(page); // Won't be removed if its not in free_matrix
                page->state = PG_TAIL;
                buddy->order += 1;
                page = buddy;
            } else break;
        }
    }

    insert_free(page);
}