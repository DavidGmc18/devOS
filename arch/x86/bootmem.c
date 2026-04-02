#include "bootmem.h"
#include <stddef.h>
#include <printk.h>
#include <kernel/panic.h>

extern char KERNEL_PHYS[];
extern char __kernel_vma_start[];
extern char __kernel_vma_end[];

#define PAGE_SIZE 4096
#define KERNEL_START (uintptr_t)KERNEL_PHYS
#define KERNEL_SIZE ((uintptr_t)__kernel_vma_end - (uintptr_t)__kernel_vma_start)

#define PALLIGN(addr) (((addr) + PAGE_SIZE -1) & ~(PAGE_SIZE-1)) // Allign to next page start

static struct e820_table* e820_table;
static void* next_addr;
static void* last_phys;

int bootmem_init(struct e820_table* e820_table_ptr) {
    e820_table = e820_table_ptr;
    next_addr = (void*)PALLIGN(KERNEL_START + KERNEL_SIZE);

    last_phys = 0;
    for (int i = 0; i < e820_table->entries_count; i++) {
        struct e820_entry* entry = e820_table->entries+i;
        if (entry->type != E820_TYPE_RAM) continue;
        uintptr_t end = entry->addr + entry->size;
        if (end > (uintptr_t)last_phys) last_phys = (void*)end;
    }

    if (!last_phys) {
        panic("Bootmem init failed: No usable RAM found in E820 table!\n");
        return -1;
    }

    printk("[OK] Bootmem initialized. Range: 0x%p - 0x%p\n", next_addr, last_phys);
    return 0;
}

void* bootmem_alloc(int count) {
    uint64_t size = (uint64_t)count * PAGE_SIZE;
    uintptr_t start = PALLIGN((uintptr_t)next_addr);
    uintptr_t end = PALLIGN(start + size);

    while (end <= (uintptr_t)last_phys) {
        int is_ram = 0;
        int collision = 0; 
        end = PALLIGN(start + size);

        uintptr_t next_ram_block = UINTPTR_MAX;

        for (int i = 0; i < e820_table->entries_count; i++) {
            struct e820_entry* entry = e820_table->entries+i;

            if (entry->type == E820_TYPE_RAM) {
                if (!is_ram && start >= entry->addr && end <= entry->addr + entry->size) is_ram = 1;
                if (!is_ram && entry->addr > start && entry->addr < next_ram_block) {
                    next_ram_block = entry->addr;
                }
            } else if (start < entry->addr + entry->size && entry->addr < end) {
                start = PALLIGN(entry->addr + entry->size);
                collision = 1;
                break;
            }
        }

        if (collision) continue;

        if (is_ram && !collision) {
            next_addr = (void*)PALLIGN(start + size);
            return (void*)start;
        }

        if (next_ram_block != UINTPTR_MAX) start = PALLIGN(next_ram_block); else break;
    }
    
    return NULL;
}