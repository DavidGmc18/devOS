#pragma once

#include <stddef.h>
#include <boot/BootParams.h>
#include "OwnedPageBlock.hpp"

typedef uintptr_t PageIndex;

class PageAllocator {
public:
    static constexpr size_t PAGE_SIZE = 4096;

private:
    static bool initialized;

    static uintptr_t last_address;
    static size_t total_pages;
    static uint8_t* const bitmap;

    static uintptr_t usable_mem;

    static bool get(PageIndex page);
    static void set(PageIndex page, bool value);
    static void set_block(PageIndex page, size_t num_pages, bool value);

    static size_t count_allocated_pages();

    static PageIndex find_block(size_t num_pages);

public:
    PageAllocator(const MemoryInfo* mem_info);

    static uintptr_t get_last_address();
    static uintptr_t get_usable_memory();
    static uintptr_t get_used_memory(); // including reserved memory
    static uintptr_t get_free_memory();

    static OwnedPageBlock alloc(size_t num_pages);
    static void free(OwnedPageBlock& block);
};