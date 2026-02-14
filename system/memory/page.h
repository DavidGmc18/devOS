#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct {
    void* const ptr;
    const uintptr_t count;
} PageBlock;

PageBlock alloc_page(uintptr_t count);
void free_page(PageBlock* block);

#ifdef __cplusplus
}
#endif