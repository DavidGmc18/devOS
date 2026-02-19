#pragma once

#include <stddef.h>

class OwnedPageBlock final {
    friend class PageAllocator;

protected:
    void* _address;
    size_t _page_count;

    OwnedPageBlock(void* addr, size_t count);

private:
    OwnedPageBlock& operator=(OwnedPageBlock&&);

public:
    int alloc(size_t num_pages);
    void free();

    ~OwnedPageBlock();

    const void* const& address;
    const size_t& page_count;

    OwnedPageBlock(OwnedPageBlock&& other);

    OwnedPageBlock(const OwnedPageBlock& other) = delete;
    OwnedPageBlock& operator=(const OwnedPageBlock&) = delete;
};