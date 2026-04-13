#include "bootmem.h"
#include <printk.h>
#include <panic.h>
#include "e820.h"
#include <math.h>
#include "vmm.h"
#include <page.h>

extern char KERNEL_PHYS[];
extern char __kernel_vma_start[];
extern char __kernel_vma_end[];

#define KERNEL_BASE (uintptr_t)KERNEL_PHYS
#define KERNEL_SIZE ((uintptr_t)__kernel_vma_end - (uintptr_t)__kernel_vma_start)
#define KERNEL_END (KERNEL_BASE + KERNEL_SIZE)

#define HHDM_BASE (0xFFFF888000000000)

static uintptr_t pool_start;
static uintptr_t pool_end;
static uint32_t current_e820_entry;
static size_t pool_bytes;

enum {
    DISABLED,
    ENABLED,
    LOCKED
} state = DISABLED;

int bootmem_init() {
    if (state != DISABLED) {
        printk(KERN_ERR"[ERR] Bootmem was alredy initialized\n");
        return -1;
    }

    pool_start = 0;
    pool_end = 0;
    pool_bytes = 0;

    struct e820_table* table = e820_get_table();
    uint32_t entries_count = table->entries_count;
    struct e820_entry* entries = table->entries;

    for (uint32_t i = 0; i < entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;
        if (entries[i].addr + entries[i].size <= KERNEL_END) continue;

        pool_start = (entries[i].addr < KERNEL_END) ? KERNEL_END : entries[i].addr;
        current_e820_entry = i;
        break;
    }

    if (!pool_start) panic("Bootmem failed to create pool!\n");
    if (pool_start > vmm_get_low_map_end()) panic("Bootmem failed to create pool!\n");

    pool_start = ROUND_UP(pool_start, PAGE_SIZE);
    pool_end = pool_start;

    state = ENABLED;
    printk("[OK] Bootmem created pool\n");
    return 0;
}

uintptr_t bootmem_alloc_page_phys() {
    if (state != ENABLED) {
        if (state == DISABLED) printk(KERN_ERR"[ERR] Bootmem was not initialized, can't alloc\n");
        if (state == LOCKED) printk(KERN_ERR"[ERR] Bootmem is locked, can't alloc\n");
        return 0;
    }

    struct e820_table* table = e820_get_table();
    uint32_t entries_count = table->entries_count;
    struct e820_entry* entries = table->entries;

    if (current_e820_entry >= entries_count) panic("Bootmem state corrupted!\n");
    if (entries[current_e820_entry].addr > pool_end) panic("Bootmem state corrupted!\n");

    uintptr_t allocation = ROUND_UP(pool_end, PAGE_SIZE);

    for (uint32_t i = current_e820_entry; i < entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;
        if (allocation < entries[i].addr) allocation = ROUND_UP(entries[i].addr, PAGE_SIZE);
        if (allocation + PAGE_SIZE > entries[i].addr + entries[i].size) continue;

        pool_end = (allocation + PAGE_SIZE);
        current_e820_entry = i;
        pool_bytes += PAGE_SIZE;
        return allocation;
    }
    
    return 0;
}

void* bootmem_alloc_page() {
    uintptr_t phys = bootmem_alloc_page_phys();
    if (!phys) return NULL;
    return (void*)(HHDM_BASE + phys);
}

uintptr_t bootmem_get_pool_start() {
    return pool_start;
}

uintptr_t bootmem_get_pool_end() {
    return pool_end;
}

uint64_t bootmem_get_pool_bytes() {
    return pool_bytes;
}

struct alloc_segment {
    size_t size;
    struct alloc_segment* next;
};

static struct alloc_segment* __alloc(size_t bytes) {
    if (state != ENABLED) {
        if (state == DISABLED) printk(KERN_ERR"[ERR] Bootmem was not initialized, can't alloc\n");
        if (state == LOCKED) printk(KERN_ERR"[ERR] Bootmem is locked, can't alloc\n");
        return NULL;
    }
    if (!bytes) return NULL;

    size_t allocated = 0;
    struct alloc_segment* allocation = NULL;
    struct alloc_segment* current_segment = NULL;

    struct e820_table* table = e820_get_table();
    uint32_t entries_count = table->entries_count;
    struct e820_entry* entries = table->entries;

    for (uint32_t i = current_e820_entry; i < entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;

        uintptr_t segment_end = ROUND_DOWN(entries[i].addr + entries[i].size, PAGE_SIZE);

        uintptr_t segment_start = (pool_end > entries[i].addr) ? pool_end : entries[i].addr;
        segment_start = ROUND_UP(segment_start, PAGE_SIZE);
        if (segment_start >= segment_end) continue;

        size_t remaining = bytes - allocated;
        if (segment_start + remaining < segment_end) segment_end = segment_start + remaining;
        
        size_t segment_size = segment_end - segment_start;

        struct alloc_segment* prev_segment = current_segment;
        current_segment = (struct alloc_segment*)(segment_start + HHDM_BASE);
        current_segment->size = segment_size;
        current_segment->next = NULL;
        if (prev_segment) prev_segment->next = current_segment;
        if (!allocation) allocation = current_segment;

        allocated += segment_size;
        if (allocated < bytes) continue;

        pool_end = segment_end;
        pool_bytes += bytes;
        current_e820_entry = i;
        return allocation;
    }

    printk(KERN_WARNING "[WARN] Allocation failed, requested %lld KiB but found only %lld KiB\n", bytes / 1024, allocated / 1024);
    return NULL;
}

void* bootmem_alloc_mapped(uintptr_t virt, size_t bytes, uintptr_t flags) {
    if (state != ENABLED) {
        if (state == DISABLED) printk(KERN_ERR"[ERR] Bootmem was not initialized, can't alloc\n");
        if (state == LOCKED) printk(KERN_ERR"[ERR] Bootmem is locked, can't alloc\n");
        return NULL;
    }

    if (!virt) return NULL;

    if (!bytes) return NULL;
    bytes = ROUND_UP(bytes, PAGE_SIZE);

    if (vmm_virt_range_has_mapping(virt, bytes)) return NULL;

    struct alloc_segment* segment = __alloc(bytes);
    if (!segment) return NULL;

    size_t failed = 0;

    size_t offset = 0;
    while (segment) {
        uintptr_t phys = (uintptr_t)segment - HHDM_BASE;
        failed = vmm_map(virt + offset, phys, segment->size, flags);
        offset += segment->size;
        segment = segment->next;
    }

    if (failed) panic("Bootmem map alloc failed, can't recover\n");
    if (offset != bytes) panic("Bootmem map alloc failed, can't recover\n");

    return (void*)virt;
}

void bootmem_lock() {
    state = LOCKED;
    printk(KERN_INFO "[INFO] Bootmem locked, pool size is %zu KiB\n", pool_bytes / 1024);
}