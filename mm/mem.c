#include "mem.h"
#include "page.h"
#include <arch/x86/e820.h>
#include <math.h>
#include <stddef.h>
#include <arch/x86/bootmem.h>
#include <arch/x86/vmm.h>
#include <panic.h>

#define MEM_MAP_ADDR (0xFFFFEA0000000000)
static struct page* mem_map;
static size_t total_pages;

int mem_init() {
    struct e820_table* table = e820_get_table();
    struct e820_entry* entries = table->entries;
    uint32_t entries_count = table->entries_count;
    
    uintptr_t last_addr = e820_get_last_ram_addr();
    total_pages = last_addr / PAGE_SIZE;

    mem_map = (struct page*)bootmem_alloc_mapped(MEM_MAP_ADDR, total_pages * sizeof(struct page), PT_PRESENT | PT_GLOBAL | PT_NX | PT_WRITABLE);
    if (!mem_map) panic("Memory map module failed to allocate\n");

    bootmem_lock();
    uintptr_t kernel_space_end = ROUND_UP(bootmem_get_pool_end(), PAGE_SIZE);
    size_t kernel_space_pages = kernel_space_end / PAGE_SIZE;

    for (uintptr_t i = 0; i < total_pages; i++) {
        mem_map[i].flags = PAGE_RESERVED;
    } 

    return 0;
}