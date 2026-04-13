#pragma once

#include <stddef.h>

extern struct page* mem_map;
extern size_t mem_nr_pages;

int mem_init();

int buddy_init();

struct page* alloc_pages(unsigned long order);
void free_pages(struct page* page);