#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <boot/BootParams.h>

#define PAGE_SIZE 4096
#define INVALID_PAGE 0

typedef struct {
    void* const address;
    const uintptr_t page_count;
} PAGE_Block; 

void PAGE_initialize(const MemoryInfo* mem_info);

PAGE_Block PAGE_alloc(uintptr_t num_pages);
void PAGE_free(PAGE_Block* page_block);

void* PAGE_address(PAGE_Block* block);
uintptr_t PAGE_page_count(PAGE_Block* block);

uintptr_t PAGE_total_mem();
uintptr_t PAGE_usable_mem();
uintptr_t PAGE_unused_mem();

#ifdef __cplusplus
}
#endif