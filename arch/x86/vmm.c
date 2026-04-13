#include "vmm.h"
#include <printk.h>
#include <string.h>
#include "e820.h"
#include "math.h"
#include "bootmem.h"
#include <stdbool.h>
#include <page.h>

// TODO add huge pages

extern char __kernel_vma_start[];
extern char KERNEL_PHYS[];
#define HHDM_BASE (0xFFFF888000000000)

#define FLAGS_MASK (0x8000000000000FFF)
#define LARGE_PAGE_SIZE (0x200000)
#define LARGE_FLAGS_MASK (0x80000000001FFFFF)

static enum state {
    VMM_EARLY = 0,
    VMM_LOW_MAP = 1,
    VMM_TRANS = 2,
    VMM_HHDM = 3
} state = VMM_EARLY;
static uintptr_t low_map_end;

static __attribute__((aligned(4096))) uint64_t kern_pml4[512];

static __attribute__((aligned(4096))) uint64_t pdpt_low[512];
static __attribute__((aligned(4096))) uint64_t pd_low[512];

static __attribute__((aligned(4096))) uint64_t pdpt_kern[512];
static __attribute__((aligned(4096))) uint64_t pd_kern[512];

static inline uintptr_t table_to_phys(uint64_t* virt) {
    if ((uintptr_t)virt >= (uintptr_t)__kernel_vma_start) return (uintptr_t)virt + (uintptr_t)KERNEL_PHYS - (uintptr_t)__kernel_vma_start;
    if ((uintptr_t)virt >= HHDM_BASE) return ((uint64_t)virt - HHDM_BASE) & ~FLAGS_MASK;
    return (uint64_t)virt & ~FLAGS_MASK;
}

static inline void set_cr3(uintptr_t pml4) {
    __asm__ volatile(
        "mov %0, %%cr3"
        :: "r" (pml4)
        : "memory"
    );
}

static void map_low_identity() {
    kern_pml4[0] = table_to_phys(pdpt_low) | PT_PRESENT | PT_WRITABLE;
    pdpt_low[0] = table_to_phys(pd_low) | PT_PRESENT | PT_WRITABLE;
    for (uintptr_t i = 0; i < 512; i++) {
        pd_low[i] = (i*0x200000) | PT_PRESENT | PT_WRITABLE | PT_PSE;
    }

    state = VMM_LOW_MAP;
    low_map_end = 512 * 0x200000;
}

static void map_kernel() {
    uint16_t idx = ((uintptr_t)__kernel_vma_start >> 39) & 0x1FF;
    kern_pml4[idx] = table_to_phys(pdpt_kern) | PT_PRESENT | PT_WRITABLE;

    idx = ((uintptr_t)__kernel_vma_start >> 30) & 0x1FF;
    pdpt_kern[idx] = table_to_phys(pd_kern) | PT_PRESENT | PT_WRITABLE;

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

    set_cr3(table_to_phys(kern_pml4));

    printk("[OK] VMM initialized\n");
}

uintptr_t vmm_get_low_map_end() {
    return low_map_end;
}

static inline uint64_t* phys_to_table(uintptr_t table) {
    if (state >= VMM_TRANS) return (uint64_t*)(table + HHDM_BASE);
    return (uint64_t*)table;
}

static uint64_t* get_or_create_table(uint64_t* table, uint32_t idx) {
    if (state < VMM_LOW_MAP) return NULL;
    if (idx >= 512) return NULL;

    uint64_t entry = table[idx];

    if (entry & PT_PRESENT) {
        if (entry & PT_PSE) return NULL;
        return phys_to_table(entry & ~FLAGS_MASK);
    }

    uint64_t* sub_table = (state == VMM_LOW_MAP) ? (uint64_t*)bootmem_alloc_page_phys() : (uint64_t*)bootmem_alloc_page();
    if (!sub_table) return NULL;
    memset(sub_table, 0, PAGE_SIZE);
    table[idx] = table_to_phys(sub_table) | PT_PRESENT | PT_WRITABLE;
    return sub_table;
}

static uint64_t* get_or_create_pt(uintptr_t virt) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(kern_pml4, idx);
    if (!pdpt) return NULL;

    idx = (virt >> 30) & 0x1FF;
    uint64_t* pd = get_or_create_table(pdpt, idx);
    if (!pd) return NULL;

    idx = (virt >> 21) & 0x1FF;
    return get_or_create_table(pd, idx);
}

static uint64_t* get_or_create_pd(uintptr_t virt) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(kern_pml4, idx);
    if (!pdpt) return NULL;

    idx = (virt >> 30) & 0x1FF;
    return get_or_create_table(pdpt, idx);
}

static int set_page(uintptr_t virt, uintptr_t phys, uintptr_t flags, bool overwrite) {
    if (virt & (PAGE_SIZE - 1)) return -1;
    if (phys & (PAGE_SIZE - 1)) return -1;

    uint64_t* pt = get_or_create_pt(virt);
    if (!pt) return -1;

    uint16_t idx = (virt >> 12) & 0x1FF;
    if (!overwrite && (pt[idx] & PT_PRESENT)) return -1;
    pt[idx] = (phys & ~FLAGS_MASK) | flags;
    return 0;
}

static int set_large_page(uintptr_t virt, uintptr_t phys, uintptr_t flags, bool overwrite) {
    if (virt & (LARGE_PAGE_SIZE - 1)) return -1;
    if (phys & (LARGE_PAGE_SIZE - 1)) return -1;

    uint64_t* pd = get_or_create_pd(virt);
    if (!pd) return -1;

    uint16_t idx = (virt >> 21) & 0x1FF;
    if (!overwrite && (pd[idx] & PT_PRESENT)) return -1;
    pd[idx] = (phys & ~LARGE_FLAGS_MASK) | flags | PT_PSE;
    return 0;
}

static size_t set_pages(uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
    if (virt & (PAGE_SIZE - 1)) return size;
    if (phys & (PAGE_SIZE - 1)) return size;
    if (size & (PAGE_SIZE - 1)) return size;

    size_t failed = 0;

    for (size_t i = 0; i < size; i += PAGE_SIZE) {
        int code = set_page(virt + i, phys + i, flags, overwrite);
        if (code) failed += PAGE_SIZE;
    }

    return failed;
}

static size_t set_large_pages(uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
    if (virt & (LARGE_PAGE_SIZE - 1)) return size;
    if (phys & (LARGE_PAGE_SIZE - 1)) return size;
    if (size & (LARGE_PAGE_SIZE - 1)) return size;

    size_t failed = 0;
    for (size_t i = 0; i < size; i += LARGE_PAGE_SIZE) {
        int code = set_large_page(virt + i, phys + i, flags, overwrite);
        if (code) failed += LARGE_PAGE_SIZE;
    }

    return failed;
}

static size_t set_range(uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
    if ((virt & (LARGE_PAGE_SIZE - 1)) != (phys & (LARGE_PAGE_SIZE - 1))) return size;

    size_t failed = 0;
    uintptr_t virt_end = virt + size;

    if (virt & (PAGE_SIZE - 1)) {
        uintptr_t offset = ROUND_UP(virt, PAGE_SIZE) - virt;
        virt += offset;
        phys += offset;
        failed += offset;
    }

    if (virt_end & (PAGE_SIZE - 1)) {
        uintptr_t offset = virt_end - ROUND_DOWN(virt_end, PAGE_SIZE);
        virt_end -= offset;
        failed += offset;
    }

    if (virt >= virt_end) return size;

    uintptr_t large_virt = ROUND_UP(virt, LARGE_PAGE_SIZE);
    uintptr_t large_phys = ROUND_UP(phys, LARGE_PAGE_SIZE);
    uintptr_t large_virt_end = ROUND_DOWN(virt_end, LARGE_PAGE_SIZE);
    uintptr_t large_size = (large_virt_end > large_virt) ? (large_virt_end - large_virt) : 0;

    uintptr_t pre_pad_end  = (large_virt < virt_end) ? large_virt : virt_end;
    uintptr_t pre_pad_size = (pre_pad_end > virt) ? (pre_pad_end - virt) : 0;

    uintptr_t pos_pad_virt = large_virt + large_size;
    uintptr_t pos_pad_phys = large_phys + large_size;
    uintptr_t pos_pad_size = (virt_end > pos_pad_virt) ? (virt_end - pos_pad_virt) : 0;

    if (pre_pad_size) failed += set_pages(virt, phys, pre_pad_size, flags, overwrite);
    if (large_size) failed += set_large_pages(large_virt, large_phys, large_size, flags, overwrite);
    if (pos_pad_size) failed += set_pages(pos_pad_virt, pos_pad_phys, pos_pad_size, flags, overwrite);

    return failed;
}

int vmm_map_hhdm() {
    if (state != VMM_LOW_MAP) {
        printk(KERN_ERR "[ERR] Low identity is not present, can't perform HHDM mapping\n");
        return -1;
    }

    uint64_t entries_count = e820_get_table()->entries_count;
    struct e820_entry* entries = e820_get_table()->entries;

    size_t failed = 0;
    size_t total = 0;

    for (int i = 0; i < entries_count; i++) {
        uint64_t flags = PT_PRESENT | PT_GLOBAL | PT_NX;
        switch (entries[i].type) {
            case E820_TYPE_RAM:
                flags |= PT_WRITABLE;
                break;

            case E820_TYPE_NVS:
            case E820_TYPE_ACPI:
                break;

            case E820_TYPE_RESERVED:
            case E820_TYPE_UNUSABLE:
            default: 
                flags = PT_PCD | PT_PWT;
            break;
        }

        failed +=set_range(HHDM_BASE + entries[i].addr, entries[i].addr, entries[i].size, flags, 0);
        total += entries[i].size;
    }

    size_t vga_failed = set_range(HHDM_BASE + 0xA0000, 0xA0000, 0x20000, PT_PRESENT | PT_WRITABLE | PT_GLOBAL | PT_NX | PT_PCD | PT_PWT, 0);
    if (vga_failed) printk(KERN_WARNING "[WARN] Failed to fully map VGA framebuffer\n");

    state = VMM_TRANS;

    if (failed) {
        printk(KERN_WARNING "[WARN] HHDM mapped %llu KiB and skipped %llu KiB\n", (total - failed) / 1024, failed / 1024);
        return -1;
    }
    
    printk("[OK] HHDM mapped %llu KiB\n", total / 1024);
    return 0;
}

int vmm_unmap_low_identity() {
    if (state != VMM_TRANS) {
        printk(KERN_ERR, "[ERR] VMM failed to unmap low identity\n\n");
        return -1;
    }
    kern_pml4[0] = 0;
    __asm__ volatile(
        "mov %%cr3, %%rax;"
        "mov %%rax, %%cr3"
        ::: "rax", "memory"
    ); // flush TLB
    state = VMM_HHDM;
    printk("[OK] VMM unmapped low identity\n");
    return 0;
}

size_t vmm_map(uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags) {
    if ((virt & (LARGE_PAGE_SIZE - 1)) != (phys & (LARGE_PAGE_SIZE - 1))) {
        if ((virt & (PAGE_SIZE - 1)) != (phys & (PAGE_SIZE - 1))) return size;
        size_t failed = 0;

        uintptr_t virt_end = virt + size;

        if (virt & (PAGE_SIZE - 1)) {
            uintptr_t offset = ROUND_UP(virt, PAGE_SIZE) - virt;
            virt += offset;
            phys += offset;
            failed += offset;
        }

        if (virt_end & (PAGE_SIZE - 1)) {
            uintptr_t offset = virt_end - ROUND_DOWN(virt_end, PAGE_SIZE);
            virt_end -= offset;
            failed += offset;
        }

        if (virt >= virt_end) return size;
        size = virt_end - virt;

        failed += set_pages(virt, phys, size, flags, 0);
        return failed;
    }
    return set_range(virt, phys, size, flags, 0);
}

static int is_pt_mapped(uint64_t* pt, uint32_t start_idx, uint32_t entries_count) {
    if (!pt) return -1;

    uint32_t end_idx = start_idx + entries_count;
    if (end_idx > 512) end_idx = 512;
    
    for (uint32_t i = start_idx; i < end_idx; i++) {
        uint64_t entry = pt[i];
        if (!(entry & PT_PRESENT)) continue;
        return 1;
    }

    return 0;
}

int vmm_virt_range_has_mapping(uintptr_t virt, size_t size) {
    uintptr_t end = virt + size;

    while (virt < end) {
        uint16_t idx = (virt >> 39) & 0x1FF;
        uint64_t entry = kern_pml4[idx];
        if (!(entry & PT_PRESENT)) { 
            virt = ROUND_DOWN(virt, 0x8000000000) + 0x8000000000;
            continue; 
        }

        uint64_t* pdpt = phys_to_table(entry & ~FLAGS_MASK);
        idx = (virt >> 30) & 0x1FF;
        entry = pdpt[idx];
        if (!(entry & PT_PRESENT)) {
            virt = ROUND_DOWN(virt, 0x40000000) + 0x40000000;
            continue;
        }
        if (entry & PT_PSE) return 1;

        uint64_t* pd = phys_to_table(entry & ~FLAGS_MASK);
        idx = (virt >> 21) & 0x1FF;
        entry = pd[idx];
        if (!(entry & PT_PRESENT)) {
            virt = ROUND_DOWN(virt, LARGE_PAGE_SIZE) + LARGE_PAGE_SIZE;
            continue;
        }
        if (entry & PT_PSE) return 1;

        uint64_t* pt = phys_to_table(entry & ~FLAGS_MASK);
        idx = (virt >> 12) & 0x1FF;
        entry = pt[idx];
        if (!(entry & PT_PRESENT)) {
            virt += PAGE_SIZE;
            continue;
        }
        return 1;
    }

    return 0;
}