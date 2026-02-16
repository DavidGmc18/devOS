#pragma once

#include "page.h"

class PageOwner {
    void* address;
    uintptr_t page_count;

public:
    PageOwner();
    ~PageOwner();

    int alloc(uintptr_t num_pages);
    void free();

    void* get_address() const;
    uintptr_t get_page_count() const;

    PageOwner(const PageOwner&) = delete;
    PageOwner& operator=(const PageOwner&) = delete;
};