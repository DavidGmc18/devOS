#include "page.hpp"
#include <stddef.h>

PageOwner::PageOwner(): address(NULL), page_count(0) {}

PageOwner::~PageOwner() {
    free();
}

int PageOwner::alloc(uintptr_t num_pages) {
    PAGE_Block block = PAGE_alloc(num_pages);
    if (block.address == NULL || block.page_count == 0)
        return -1;

    address = block.address;
    page_count = block.page_count;
    return 0;
}

void PageOwner::free() {
    PAGE_Block block = {address, page_count};
    PAGE_free(&block);
    address = NULL;
    page_count = 0;
}


void* PageOwner::get_address() const {
    return address;
}

uintptr_t PageOwner::get_page_count() const {
    return page_count;
}