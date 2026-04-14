#pragma once

#include <page.h>

struct page* alloc_pages(unsigned char order);
void free_pages(struct page* page);