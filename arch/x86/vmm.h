#pragma once

#include <stdint.h>

void vmm_init();
uintptr_t vmm_get_early_addr_limit();

int vmm_map_hhdm();

void vmm_unmap_low_identity();