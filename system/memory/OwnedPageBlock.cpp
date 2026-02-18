#include "OwnedPageBlock.hpp"
#include "PageAllocator.hpp"

OwnedPageBlock::OwnedPageBlock(void* addr, size_t count)
    : _address(addr), _page_count(count),
    address(_address), page_count(_page_count)
{}

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