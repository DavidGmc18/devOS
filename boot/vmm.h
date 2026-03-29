#pragma once

#include <stdint.h>

int vmm_init(uint64_t kern_phys, uint64_t kern_virt);
void* vmm_get_pml4();