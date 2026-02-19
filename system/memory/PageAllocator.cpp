#include "PageAllocator.hpp"
#include <math.h>
#include <memory.h>

bool PageAllocator::initialized = false;

uintptr_t PageAllocator::last_address = 0;
size_t PageAllocator::total_pages = 0;
uint8_t* const PageAllocator::bitmap = (uint8_t*)0x200000;

uintptr_t PageAllocator::usable_mem = 0;

bool PageAllocator::get(PageIndex page) {
    if (page >= total_pages)
        return true; 

    uintptr_t address = page / 8;
    uint8_t offset = page % 8;
    return (bitmap[address] >> offset) & 0x1;
}   

void PageAllocator::set(PageIndex page, bool value) {
    if (page >= total_pages)
        return;

    uintptr_t address = page / 8;
    uint8_t offset = page % 8;
    if (value) {
        bitmap[address] |= (0x1 << offset);
    } else {
        bitmap[address] &= ~(0x1 << offset);
    }
}

void PageAllocator::set_block(PageIndex page, size_t num_pages, bool value) {
    for (size_t i = 0; i < num_pages; i++)
        set(page + i, value);
}

size_t PageAllocator::count_allocated_pages() {
    size_t count = 0;

    size_t i = 0;
    while (i + 31 < total_pages) {
        uint32_t pack = *(uint32_t*)(bitmap + i/8);
        count += __builtin_popcount(pack);
        i += 32;
    }

    while (i < total_pages) {
        count += get(i);
        i++;
    }
    
    return count;
}

PageIndex PageAllocator::find_block(size_t num_pages) {
    PageIndex start_page;
    size_t consecutive = 0;

    PageIndex current_page = 0;
    while (current_page < total_pages) {
        if (get(current_page)) {
            start_page = current_page + 1;
            consecutive = 0;
        } else {
            consecutive++;
            if (consecutive >= num_pages) {
                goto found;
            }
        }

        current_page++;
    }

    return 0;

found:
    return start_page;
}

PageAllocator::PageAllocator(const MemoryInfo* mem_info) {
    if (initialized)
        return;

    // Figure out last address availible and total page count
    last_address = 0;
    for (uint32_t i = 0; i < mem_info->block_count; i++) {
        MemoryBlock block = mem_info->blocks[i];
        uint32_t end = block.base + block.length;
        if (end > last_address)
            last_address = end;
    }

    total_pages = last_address / PAGE_SIZE;

    // Mark all pages as allocated/reserved
    size_t bitmap_size = DIV_UP(total_pages, 8);
    memset(bitmap, UINT8_MAX, bitmap_size);

    // Mark usable pages as free
    for (uint32_t i = 0; i < mem_info->block_count; i++) {
        MemoryBlock block = mem_info->blocks[i];
        if (block.type == E820_USABLE) {
            PageIndex start_page = block.base / PAGE_SIZE;
            PageIndex end_page = DIV_UP(block.base + block.length, PAGE_SIZE);
            set_block(start_page, end_page - start_page, 0);
        }
    }

    // In case of memory regions overlap, mark non-usable pages as allocated/reserved
    for (uint32_t i = 0; i < mem_info->block_count; i++) {
        MemoryBlock block = mem_info->blocks[i];
        if (block.type != E820_USABLE) {
            PageIndex start_page = block.base / PAGE_SIZE;
            PageIndex end_page = DIV_UP(block.base + block.length, PAGE_SIZE);
            set_block(start_page, end_page - start_page, 1);
        }
    }

    // Compute amount of usable memory
    size_t reserved_mem = count_allocated_pages() * PAGE_SIZE;
    usable_mem = last_address - reserved_mem;

    // Addresses before bitmap are reserved for kernel, bios, hardware, etc.
    // Bitmap needs to be marked allocated
    uintptr_t bitmap_end = (uintptr_t)bitmap + bitmap_size;
    size_t bitmap_end_page = DIV_UP(bitmap_end, PAGE_SIZE);
    set_block(0, bitmap_end_page, 1);

    initialized = true;
}

uintptr_t PageAllocator::get_last_address() {
    return last_address;
}

uintptr_t PageAllocator::get_usable_memory() {
    return usable_mem;
}

uintptr_t PageAllocator::get_used_memory() {
    return count_allocated_pages() * PAGE_SIZE;
}

uintptr_t PageAllocator::get_free_memory() {
    return last_address - get_used_memory();
}

OwnedPageBlock PageAllocator::alloc(size_t num_pages) {
    PageIndex page = find_block(num_pages);
    set_block(page, num_pages, 1);
    void* address = (void*)(page * PAGE_SIZE);
    OwnedPageBlock block(address, num_pages);
    return block;
}

void PageAllocator::free(OwnedPageBlock& block) {
    if (block._page_count == 0)
        return;
        
    PageIndex page = (uintptr_t)block._address / PAGE_SIZE;
    set_block(page, block._page_count, 0);
    block._address = NULL;
    block._page_count = 0;
}