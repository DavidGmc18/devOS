#include "OwnedPageBlock.hpp"
#include "PageAllocator.hpp"
#include <system/debug/logger.h>

OwnedPageBlock::OwnedPageBlock(void* addr, size_t count)
    : _address(addr), _page_count(count),
    address(_address), page_count(_page_count)
{}

OwnedPageBlock& OwnedPageBlock::operator=(OwnedPageBlock&& other) noexcept {
    if (this != &other) {
        if (_page_count > 0) {
            logf("OwnedPageBlock", LOGGER_LVL_ERROR, "Attempted to transfer ownership into non-empty block!");
            free();
        }

        _address = other._address;
        _page_count = other._page_count;

        other._address = NULL;
        other._page_count = 0;
    }

    return *this;
}

int OwnedPageBlock::alloc(size_t num_pages) {
    if (_page_count > 0) {
        logf("OwnedPageBlock", LOGGER_LVL_WARN, "Attempted to allocate into non-empty block!");
        return -1;
    }

    *this = PageAllocator::alloc(num_pages);
    if (_address != NULL && _page_count == num_pages) {
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

OwnedPageBlock::OwnedPageBlock(OwnedPageBlock&& other) noexcept
    : _address(other._address), _page_count(other._page_count),
    address(_address), page_count(_page_count)
{
    other._address = nullptr;
    other._page_count = 0;
}