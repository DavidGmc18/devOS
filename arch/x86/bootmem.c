#include "bootmem.h"
#include <stddef.h>
#include <printk.h>
#include <panic.h>

extern char KERNEL_PHYS[];
extern char __kernel_vma_start[];
extern char __kernel_vma_end[];

#define PAGE_SIZE 4096
#define KERNEL_BASE (uintptr_t)KERNEL_PHYS
#define KERNEL_SIZE ((uintptr_t)__kernel_vma_end - (uintptr_t)__kernel_vma_start)
#define KERNEL_END (KERNEL_BASE + KERNEL_SIZE)

// TODO, move to math.h? and make it better
#define ROUND_DOWN(a, b) ((a / b) * b)
#define ROUND_UP(a, b) (((a + b - 1) / b) * b)

static void* alloc_base;
static void* alloc_end;
static void* alloc_ptr;

int bootmem_init(struct e820_table* e820_table_ptr) {
    alloc_base = NULL;
    alloc_end = NULL;
    alloc_ptr = NULL;

    struct e820_entry* entries = e820_table_ptr->entries;
    uint64_t last_address = entries[e820_table_ptr->entries_count-1].addr + entries[e820_table_ptr->entries_count-1].size;
    uint64_t min_size = ((last_address + 0x1FFFFF) / 0x200000) * 0x1000; // Calculated for HHDM mapping

    for (int i = 0; i < e820_table_ptr->entries_count; i++) {
        if (entries[i].type != E820_TYPE_RAM) continue;

        uintptr_t entry_end = entries[i].addr + entries[i].size;
        entry_end = ROUND_DOWN(entry_end, PAGE_SIZE);
        if (entry_end <= KERNEL_END) continue;

        uintptr_t entry_base = (entries[i].addr >= KERNEL_END) ? entries[i].addr : KERNEL_END;
        entry_base = ROUND_UP(entry_base, PAGE_SIZE);
        if (entry_base >= entry_end) continue;

        uint64_t size = entry_end - entry_base;
        if (size <= alloc_end - alloc_base) continue;

        alloc_base = (void*)entry_base;
        alloc_end = (void*)entry_end;
        if (size >= min_size) break;
    }

    if (!alloc_base || !alloc_end || alloc_end <= alloc_base) {
        panic("Bootmem init failed: No usable RAM found in E820 table!\n");
        return -1;
    }

    alloc_ptr = alloc_base;

    printk("[OK] Bootmem initialized\n");
    return 0;
}

void* bootmem_alloc(int bytes) {
    if ((uintptr_t)alloc_ptr < (uintptr_t)alloc_base) panic("Bootmem encountered unexpected state!\n");
    uintptr_t allocation = ROUND_UP((uintptr_t)alloc_ptr, PAGE_SIZE);
    if (allocation + bytes > (uintptr_t)alloc_end) return NULL;
    alloc_ptr = (void*)(allocation + bytes);
    return (void*)allocation;
}