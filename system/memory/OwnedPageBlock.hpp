#pragma once

#include <stddef.h>

class OwnedPageBlock final {
    friend class PageAllocator;

protected:
    void* address;
    size_t page_count;

    OwnedPageBlock(void* addr, size_t count);

private:
    OwnedPageBlock& operator=(OwnedPageBlock&&);

public:
    OwnedPageBlock();

    int alloc(size_t num_pages);
    void free();

    ~OwnedPageBlock();

    void* get_address() const;
    size_t get_page_count() const;

    OwnedPageBlock(OwnedPageBlock&& other);

    OwnedPageBlock(const OwnedPageBlock& other) = delete;
    OwnedPageBlock& operator=(const OwnedPageBlock&) = delete;
};