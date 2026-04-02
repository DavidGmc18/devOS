#pragma once
#include "e820.h"

int bootmem_init(struct e820_table* e820_table_ptr);
void* bootmem_alloc(int count);