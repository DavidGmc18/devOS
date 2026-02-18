#pragma once

#include <stddef.h>

class OwnedPageBlock {
    friend class PageAllocator;

protected:
    void* _address;
    size_t _page_count;

    OwnedPageBlock(void* addr, size_t count);

public:
    void free();

    ~OwnedPageBlock();

    const void* const& address;
    const size_t& page_count;

    OwnedPageBlock(OwnedPageBlock&& other);

    OwnedPageBlock(const OwnedPageBlock& other) = delete;
    OwnedPageBlock& operator=(const OwnedPageBlock&) = delete;
    OwnedPageBlock& operator=(OwnedPageBlock&&) = delete;
};