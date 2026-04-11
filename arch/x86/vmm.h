#pragma once

#include <stdint.h>
#include <stddef.h>

#define PT_PRESENT (1ULL << 0)
#define PT_WRITABLE (1ULL << 1)
#define PT_USER (1ULL << 2) // If not set, it is Kernel-only
#define PT_PWT (1ULL << 3) // Page-level Writethrough
#define PT_PCD (1ULL << 4) // Page-level Cache Disable
#define PT_ACCESSED (1ULL << 5)
#define PT_DIRTY (1ULL << 6)
#define PT_PSE (1ULL << 7)
#define PT_GLOBAL (1ULL << 8)
#define PT_NX (1ULL << 63)// No-Execute (requires EFER.NXE)

void vmm_init();
uintptr_t vmm_get_low_map_end();
int vmm_map_hhdm();
int vmm_unmap_low_identity();

size_t vmm_map(uintptr_t virt, uintptr_t phys, uint64_t size, uintptr_t flags);