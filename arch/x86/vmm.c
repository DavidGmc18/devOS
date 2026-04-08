#include "vmm.h"
#include <printk.h>
#include <string.h>
#include "e820.h"
#include "math.h"
#include "bootmem.h"

// TODO add huge pages

extern char __kernel_vma_start[];
extern char KERNEL_PHYS[];
#define HHDM_BASE (0xFFFF888000000000)

#define PT_PRESENT (1ULL << 0)
#define PT_WRITABLE (1ULL << 1)
#define PT_USER (1ULL << 2) // If NOT set, it is Kernel-only
#define PT_PWT (1ULL << 3) // Page-level Writethrough
#define PT_PCD (1ULL << 4) // Page-level Cache Disable
#define PT_ACCESSED (1ULL << 5)
#define PT_DIRTY (1ULL << 6)
#define PT_PSE (1ULL << 7)
#define PT_GLOBAL (1ULL << 8)
#define PT_NX (1ULL << 63)// No-Execute (requires EFER.NXE)

#define PAGE_SIZE (4096)
#define FLAGS_MASK (0x8000000000000FFF)

#define LARGE_PAGE_SIZE (0x200000)
#define LARGE_FLAGS_MASK (0x80000000001FFFFF)

static uintptr_t early_addr_limit;
static int low_identity_present = 0;

static __attribute__((aligned(4096))) uint64_t kern_pml4[512];

static __attribute__((aligned(4096))) uint64_t pdpt_low[512];
static __attribute__((aligned(4096))) uint64_t pd_low[512];

static __attribute__((aligned(4096))) uint64_t pdpt_kern[512];
static __attribute__((aligned(4096))) uint64_t pd_kern[512];

static inline uintptr_t to_physical(void* addr) {
    if ((uintptr_t)addr >= (uintptr_t)__kernel_vma_start)
        return (uintptr_t)addr + (uintptr_t)KERNEL_PHYS - (uintptr_t)__kernel_vma_start;

    if ((uintptr_t)addr >= HHDM_BASE)
        return (uintptr_t)addr - HHDM_BASE;

    return (uintptr_t)addr;
}

static inline uintptr_t to_hhdm(void* addr) {
    return (uintptr_t)addr + HHDM_BASE;
}

static inline void set_cr3(uintptr_t pml4) {
    __asm__ volatile(
        "mov %0, %%cr3"
        :: "r" (pml4)
        : "memory"
    );
}

static void map_low_identity() {
    kern_pml4[0] = to_physical(pdpt_low) | PT_PRESENT | PT_WRITABLE;
    pdpt_low[0] = to_physical(pd_low) | PT_PRESENT | PT_WRITABLE;
    for (uintptr_t i = 0; i < 512; i++) {
        pd_low[i] = (i*0x200000) | PT_PRESENT | PT_WRITABLE | PT_PSE;
    }
    early_addr_limit = 512 * 0x200000;
    low_identity_present = 1;
}

static void map_kernel() {
    uint16_t idx = ((uintptr_t)__kernel_vma_start >> 39) & 0x1FF;
    kern_pml4[idx] = to_physical(pdpt_kern) | PT_PRESENT | PT_WRITABLE;

    idx = ((uintptr_t)__kernel_vma_start >> 30) & 0x1FF;
    pdpt_kern[idx] = to_physical(pd_kern) | PT_PRESENT | PT_WRITABLE;

    idx = ((uintptr_t)__kernel_vma_start >> 21) & 0x1FF;
    pd_kern[idx] = (uintptr_t)KERNEL_PHYS | PT_PRESENT | PT_WRITABLE | PT_PSE | PT_GLOBAL;
}

static void enable_nxe() {
    uint64_t efer;
    __asm__ volatile(
        "mov $0xC0000080, %%ecx\n" // EFER MSR
        "rdmsr\n"
        "or $(1 << 11), %%eax\n" // NXE bit
        "wrmsr\n"
        ::: "eax", "ecx", "edx"
    );
}

static void enable_pge() {
    __asm__ volatile(
        "mov %%cr4, %%rax\n"
        "or $(1 << 7), %%rax\n"
        "mov %%rax, %%cr4\n"
        ::: "rax"
    );
}

void vmm_init() {
    enable_nxe();
    enable_pge();

    map_low_identity();
    map_kernel();

    set_cr3(to_physical(kern_pml4));

    printk("[OK] VMM initialized\n");
}

uintptr_t vmm_get_early_addr_limit() {
    return early_addr_limit;
}

// In early 0x0-0x40000000 => 0x0-0x40000000 (non hhdm mapping)
uint64_t* get_or_create_table(uint64_t* table, uint32_t idx) {
    if (!low_identity_present) return NULL;
    if (idx >= 512) return NULL;

    table = (uint64_t*)to_physical(table);

    uint64_t entry = table[idx];
    if (entry & PT_PRESENT) {
        uint64_t* sub_table = (uint64_t*)(entry & ~FLAGS_MASK);
        if ((entry & PT_PSE) || ((uintptr_t)sub_table + PAGE_SIZE > early_addr_limit)) return NULL;
        else return sub_table;
    }

    uint64_t* sub_table = (uint64_t*)bootmem_alloc(PAGE_SIZE);
    if (!sub_table) return NULL;
    memset(sub_table, 0, PAGE_SIZE);
    table[idx] = (uint64_t)sub_table | PT_PRESENT | PT_WRITABLE;
    return sub_table;
}

// In early 0x0-0x40000000 => 0x0-0x40000000 (non hhdm mapping)
static inline int map_normal(uintptr_t virt, uintptr_t phys, uintptr_t flags) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(kern_pml4, idx);
    if (!pdpt) return -1;

    idx = (virt >> 30) & 0x1FF;
    uint64_t* pd = get_or_create_table(pdpt, idx);
    if (!pd) return -1;

    idx = (virt >> 21) & 0x1FF;
    uint64_t* pt = get_or_create_table(pd, idx);
    if (!pt) return -1;

    idx = (virt >> 12) & 0x1FF;
    pt[idx] = (phys & ~FLAGS_MASK) | flags;
    return 0;
}

// In early 0x0-0x40000000 => 0x0-0x40000000 (non hhdm mapping)
static inline int map_large(uintptr_t virt, uintptr_t phys, uintptr_t flags) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(kern_pml4, idx);
    if (!pdpt) return -1;

    idx = (virt >> 30) & 0x1FF;
    uint64_t* pd = get_or_create_table(pdpt, idx);
    if (!pd) return -1;

    idx = (virt >> 21) & 0x1FF;
    if (pd[idx] & PT_PRESENT) return -1;
    pd[idx] = (phys & ~LARGE_FLAGS_MASK) | flags | PT_PSE;
    return 0;
}

// In early 0x0-0x40000000 => 0x0-0x40000000 (non hhdm mapping)
static inline void map_range_normal(uintptr_t base, uintptr_t end, uintptr_t flags, uint64_t* mapped, uint64_t* failed) {
    uintptr_t normal_start = ROUND_UP(base, PAGE_SIZE);
    uintptr_t normal_end = ROUND_DOWN(end, PAGE_SIZE);

    if (normal_start >= normal_end) {
        *failed += end - base;
        return;
    }  

    uintptr_t phys = normal_start;
    while (phys + PAGE_SIZE <= normal_end) {
        uintptr_t virt = HHDM_BASE + phys;
        if (map_normal(virt, phys, flags)) {
            *failed += PAGE_SIZE;
        } else {
            *mapped += PAGE_SIZE;
        }
        phys += PAGE_SIZE;
    }

    *failed += normal_start - base;
    *failed += end - normal_end;
}

// In early 0x0-0x40000000 => 0x0-0x40000000 (non hhdm mapping)
static inline void map_range_large(uintptr_t base, uintptr_t end, uintptr_t flags, uint64_t* mapped, uint64_t* failed) {
    uintptr_t large_start = ROUND_UP(base, LARGE_PAGE_SIZE);
    uintptr_t large_end = ROUND_DOWN(end, LARGE_PAGE_SIZE);

    if (large_start >= large_end) {
        map_range_normal(base, end, flags, mapped, failed);
        return;
    }    

    map_range_normal(base, large_start, flags, mapped, failed);

    uintptr_t phys = large_start;
    while (phys + LARGE_PAGE_SIZE <= large_end) {
        uintptr_t virt = HHDM_BASE + phys;
        if (map_large(virt, phys, flags)) {
            *failed += LARGE_PAGE_SIZE;
        } else {
            *mapped += LARGE_PAGE_SIZE;
        }
        phys += LARGE_PAGE_SIZE;
    }

    map_range_normal(large_end, end, flags, mapped, failed);
}

int vmm_map_hhdm() {
    if (!low_identity_present) {
        printk(KERN_ERR "[ERR] Low identity is not present, can't perform HHDM mapping\n");
        return -1;
    }

    uint64_t entries_count = e820_get_table()->entries_count;
    struct e820_entry* entries = e820_get_table()->entries;

    uint64_t mapped = 0;
    uint64_t failed = 0;

    for (int i = 0; i < entries_count; i++) {
        uint64_t flags = PT_PRESENT | PT_GLOBAL | PT_NX;
        switch (entries[i].type) {
            case E820_TYPE_RAM:
            case E820_TYPE_NVS:
                flags |= PT_WRITABLE;
                break;

            case E820_TYPE_RESERVED:
                flags |= PT_WRITABLE | PT_PCD | PT_PWT;
                break;

            case E820_TYPE_UNUSABLE:
                flags |= PT_PCD | PT_PWT;
                break;

            case E820_TYPE_ACPI:
            default: break;
        }

        map_range_large(entries[i].addr, entries[i].addr + entries[i].size, flags, &mapped, &failed);
    }

    uint64_t pre_vga_failed = failed;
    map_range_normal(0xA0000, 0xC0000, PT_PRESENT | PT_WRITABLE | PT_GLOBAL | PT_NX | PT_PCD | PT_PWT, &mapped, &failed);
    if (failed > pre_vga_failed) {
        printk(KERN_WARNING "[WARN] Failed to fully map VGA framebuffer\n");
    }

    if (failed) {
        printk(KERN_WARNING "[WARN] HHDM mapped %llu KiB and skipped %llu KiB\n", mapped / 1024, failed / 1024);
        return -1;
    }
    
    printk("[OK] HHDM mapped %llu KiB\n", mapped / 1024);
    return 0;
}

void vmm_unmap_low_identity() {
    kern_pml4[0] = 0;
    __asm__ volatile(
        "mov %%cr3, %%rax;"
        "mov %%rax, %%cr3"
        ::: "rax", "memory"
    ); // flush TLB
    low_identity_present = 0;
    printk("[OK] VMM unmapped low identity\n");
}