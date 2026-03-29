#include "vmm.h"

#define PT_PRESENT 1
#define PT_WRITABLE 2
#define PAGE_PSE (1 << 7)

static __attribute__((aligned(4096))) uint64_t pml4[512];
static __attribute__((aligned(4096))) uint64_t pdpt_l[512];
static __attribute__((aligned(4096))) uint64_t pdpt_h[512];
static __attribute__((aligned(4096))) uint64_t pd_l[512];
static __attribute__((aligned(4096))) uint64_t pd_h[512];

int vmm_init(uint64_t kern_phys, uint64_t kern_virt) {
    if (kern_virt < 0xFFFF800000000000ULL) return -1;

    // 0x000000 => 0x000000 (2MiB)
    pml4[0] = (uint64_t)(uintptr_t)pdpt_l | PT_PRESENT | PT_WRITABLE;
    pdpt_l[0] = (uint64_t)(uintptr_t)pd_l | PT_PRESENT | PT_WRITABLE;
    pd_l[0] = (uint64_t)0x0 | PT_PRESENT | PT_WRITABLE | PAGE_PSE;

    // kern_virt => kern_phys (2MiB)
    uint16_t idx = (kern_virt >> 39) & 0x1FF;
    pml4[idx] = (uint64_t)(uintptr_t)pdpt_h | PT_PRESENT | PT_WRITABLE;

    idx = (kern_virt >> 30) & 0x1FF;
    pdpt_h[idx] = (uint64_t)(uintptr_t)pd_h | PT_PRESENT | PT_WRITABLE;

    idx = (kern_virt >> 21) & 0x1FF;
    pd_h[idx] = kern_phys | PT_PRESENT | PT_WRITABLE | PAGE_PSE;

    return 0;
}

void* vmm_get_pml4() {
    return (void*)pml4;
}