#pragma once

#include <stdint.h>
#include <stddef.h>

int bootmem_init();
uintptr_t bootmem_alloc_page_phys();
void* bootmem_alloc_page();

uintptr_t bootmem_get_pool_start();
uintptr_t bootmem_get_pool_end();
uint64_t bootmem_get_pool_bytes(); // Only allocated bytes, not counting holes or padding

void* bootmem_map_alloc(uintptr_t virt, size_t bytes, uintptr_t flags);