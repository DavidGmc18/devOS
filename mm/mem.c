#include "mem.h"
#include "page.h"
#include <arch/x86/e820.h>
#include <math.h>
#include <arch/x86/bootmem.h>
#include <arch/x86/vmm.h>
#include <panic.h>
#include <printk.h>

#define MEM_MAP_ADDR (0xFFFFEA0000000000)
struct page* mem_map;
size_t mem_nr_pages;

int mem_init() {
    struct e820_table* table = e820_get_table();
    struct e820_entry* entries = table->entries;
    uint32_t entries_count = table->entries_count;
    
    uintptr_t last_addr = e820_get_last_ram_addr();
    mem_nr_pages = last_addr / PAGE_SIZE;

    mem_map = (struct page*)bootmem_alloc_mapped(MEM_MAP_ADDR, mem_nr_pages * sizeof(struct page), PT_PRESENT | PT_GLOBAL | PT_NX | PT_WRITABLE);
    if (!mem_map) panic("Memory map module failed to allocate\n");

    for (uintptr_t i = 0; i < mem_nr_pages; i++) {
        mem_map[i].flags = PG_RESERVED;
    } 

    printk("[OK] Memory map initialized\n");
    return 0;
}