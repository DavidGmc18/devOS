#include "OwnedPageBlock.hpp"
#include "PageAllocator.hpp"
#include <system/debug/logger.h>

OwnedPageBlock::OwnedPageBlock(void* addr, size_t count)
    : address(addr), page_count(count)
{}

OwnedPageBlock& OwnedPageBlock::operator=(OwnedPageBlock&& other) noexcept {
    if (this != &other) {
        if (page_count > 0) {
            logf("OwnedPageBlock", LOGGER_LVL_ERROR, "Attempted to transfer ownership into non-empty block!");
            free();
        }

        address = other.address;
        page_count = other.page_count;

        other.address = NULL;
        other.page_count = 0;
    }

    return *this;
}

OwnedPageBlock::OwnedPageBlock() 
    : address(NULL), page_count(0)
{}

int OwnedPageBlock::alloc(size_t num_pages) {
    if (page_count > 0) {
        logf("OwnedPageBlock", LOGGER_LVL_WARN, "Attempted to allocate into non-empty block!");
        return -1;
    }

    *this = PageAllocator::alloc(num_pages);
    if (address != NULL && page_count == num_pages) {
        return 0;
    } else {
        return 1;
    }
}

void OwnedPageBlock::free() {
    PageAllocator::free(*this);
}

OwnedPageBlock::~OwnedPageBlock() {
    free();
}

void* OwnedPageBlock::get_address() const {
    return address;
}

size_t OwnedPageBlock::get_page_count() const {
    return page_count;
}

OwnedPageBlock::OwnedPageBlock(OwnedPageBlock&& other) noexcept
    : address(other.address), page_count(other.page_count)
{
    other.address = nullptr;
    other.page_count = 0;
}