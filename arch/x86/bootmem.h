#pragma once

#include <stdint.h>

int bootmem_init();
uintptr_t bootmem_alloc_page_phys();
void* bootmem_alloc_page();

uintptr_t bootmem_get_pool_start();
uintptr_t bootmem_get_pool_end();
uint64_t bootmem_get_pool_bytes(); // Only allocated bytes, not counting holes or padding