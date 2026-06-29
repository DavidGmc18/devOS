#pragma once

#include <page.h>

struct page* alloc_pages(unsigned char order);
void free_pages(struct page* page);

void* page_to_addr(struct page* page);
struct page* addr_to_page(void* addr);