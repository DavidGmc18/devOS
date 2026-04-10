#include "bootmem.h"
#include <stddef.h>
#include <printk.h>
#include <panic.h>
#include "e820.h"
#include <math.h>
#include "vmm.h"

extern char KERNEL_PHYS[];
extern char __kernel_vma_start[];
extern char __kernel_vma_end[];

#define PAGE_SIZE 4096
#define KERNEL_BASE (uintptr_t)KERNEL_PHYS
#define KERNEL_SIZE ((uintptr_t)__kernel_vma_end - (uintptr_t)__kernel_vma_start)
#define KERNEL_END (KERNEL_BASE + KERNEL_SIZE)

#define HHDM_BASE (0xFFFF888000000000)

static uintptr_t pool_start;
static uintptr_t pool_end;
static uint32_t current_e820_entry;
static uint64_t pool_bytes;

int bootmem_init() {
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
    if (pool_start > vmm_get_early_addr_limit()) panic("Bootmem failed to create pool!\n");

    pool_start = ROUND_UP(pool_start, PAGE_SIZE);
    pool_end = pool_start;

    printk("[OK] Bootmem created pool\n");
    return 0;
}

uintptr_t bootmem_alloc_page_phys() {
    struct e820_table* table = e820_get_table();
    uint32_t entries_count = table->entries_count;
    struct e820_entry* entries = table->entries;

    uintptr_t allocation = ROUND_UP(pool_end, PAGE_SIZE);

    if (current_e820_entry >= entries_count) panic("Bootmem state corrupted!\n");
    if (entries[current_e820_entry].addr > allocation) panic("Bootmem state corrupted!\n");

    for (uint32_t i = current_e820_entry; i < entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;
        if (allocation < entries[i].addr) allocation = entries[i].addr;
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