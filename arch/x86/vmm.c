#include "vmm.h"
#include <printk.h>
#include <string.h>
#include "e820.h"
#include "math.h"
#include "bootmem.h"
#include <stdbool.h>
#include <page.h>
#include <mm.h>
#include <panic.h>

// TODO add huge pages

extern char __kernel_vma_start[];
extern char KERNEL_PHYS[];
#define HHDM_BASE (0xFFFF888000000000)

#define FLAGS_MASK (0x8000000000000FFF)
#define LARGE_PAGE_SIZE (0x200000)
#define LARGE_FLAGS_MASK (0x80000000001FFFFF)

#define CR3_ADDR_MASK 0x000FFFFFFFFFF000

static enum state {
    VMM_EARLY = 0,
    VMM_IDENTITY = 1, // Low mapping 1:1 for boot code
    VMM_TRANSITION = 2, // Low Map and HHDM active
    VMM_VIRTUAL_ONLY = 3, // HHDM only and bootmem alloc
    VMM_READY = 4 // HHDM only and buddy alloc (standard booted kernel)
} state = VMM_EARLY;
static uintptr_t low_map_end;

static __attribute__((aligned(4096))) uint64_t kern_pml4[512];

static __attribute__((aligned(4096))) uint64_t pdpt_low[512];
static __attribute__((aligned(4096))) uint64_t pd_low[512];

static __attribute__((aligned(4096))) uint64_t pdpt_kern[512];
static __attribute__((aligned(4096))) uint64_t pd_kern[512];

static inline uintptr_t virt_to_phys(uint64_t* virt) {
    if ((uintptr_t)virt >= (uintptr_t)__kernel_vma_start) return (uintptr_t)virt + (uintptr_t)KERNEL_PHYS - (uintptr_t)__kernel_vma_start;
    if ((uintptr_t)virt >= HHDM_BASE) return ((uint64_t)virt - HHDM_BASE) & ~FLAGS_MASK;
    return (uint64_t)virt & ~FLAGS_MASK;
}

static inline uint64_t* phys_to_virt(uintptr_t phys) {
    if (state >= VMM_TRANSITION) return (uint64_t*)(phys + HHDM_BASE);
    return (uint64_t*)phys;
}

static inline void set_cr3(uintptr_t pml4) {
    __asm__ volatile(
        "mov %0, %%cr3"
        :: "r" (pml4)
        : "memory"
    );
}

static inline uintptr_t get_cr3() {
    uintptr_t cr3;
    __asm__ volatile(
        "mov %%cr3, %0"
        : "=r" (cr3)
        :: "memory"
    );
    return cr3;
}

static void map_low_identity() {
    kern_pml4[0] = virt_to_phys(pdpt_low) | PT_PRESENT | PT_WRITABLE;
    pdpt_low[0] = virt_to_phys(pd_low) | PT_PRESENT | PT_WRITABLE;
    for (uintptr_t i = 0; i < 512; i++) {
        pd_low[i] = (i*0x200000) | PT_PRESENT | PT_WRITABLE | PT_PSE;
    }

    state = VMM_IDENTITY;
    low_map_end = 512 * 0x200000;
}

static void map_kernel() {
    uint16_t idx = ((uintptr_t)__kernel_vma_start >> 39) & 0x1FF;
    kern_pml4[idx] = virt_to_phys(pdpt_kern) | PT_PRESENT | PT_WRITABLE;

    idx = ((uintptr_t)__kernel_vma_start >> 30) & 0x1FF;
    pdpt_kern[idx] = virt_to_phys(pd_kern) | PT_PRESENT | PT_WRITABLE;

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

    set_cr3(virt_to_phys(kern_pml4));

    printk("[OK] VMM initialized\n");
}

uintptr_t vmm_get_low_map_end() {
    return low_map_end;
}

static uint64_t* __alloc_table() {
    if (state == VMM_IDENTITY) {
        return (uint64_t*)bootmem_alloc_page_phys();
    } else if (state == VMM_TRANSITION || state == VMM_VIRTUAL_ONLY) {
        return (uint64_t*)bootmem_alloc_page();
    } else if (state == VMM_READY) {
        struct page* page = alloc_pages(0);
        return (uint64_t*)page_to_addr(page);
    }
    printk(KERN_ERR "No present allocator for VMM, can't alloc\n");
    return NULL;
}

static uint64_t* get_or_create_table(uint64_t* table, uint32_t idx) {
    if (state < VMM_IDENTITY) return NULL;
    if (idx >= 512) return NULL;

    uint64_t entry = table[idx];

    if (entry & PT_PRESENT) {
        if (entry & PT_PSE) return NULL;
        return phys_to_virt(entry & ~FLAGS_MASK);
    }

    uint64_t* sub_table = __alloc_table();
    if (!sub_table) return NULL;
    memset(sub_table, 0, PAGE_SIZE);
    table[idx] = virt_to_phys(sub_table) | PT_PRESENT | PT_WRITABLE | PT_USER;
    return sub_table;
}

static uint64_t* get_or_create_pt(uint64_t* pml4, uintptr_t virt) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(pml4, idx);
    if (!pdpt) return NULL;

    idx = (virt >> 30) & 0x1FF;
    uint64_t* pd = get_or_create_table(pdpt, idx);
    if (!pd) return NULL;

    idx = (virt >> 21) & 0x1FF;
    return get_or_create_table(pd, idx);
}

static uint64_t* get_or_create_pd(uint64_t* pml4, uintptr_t virt) {
    uint16_t idx = (virt >> 39) & 0x1FF;
    uint64_t* pdpt = get_or_create_table(pml4, idx);
    if (!pdpt) return NULL;

    idx = (virt >> 30) & 0x1FF;
    return get_or_create_table(pdpt, idx);
}

static int set_page(uint64_t* pml4, uintptr_t virt, uintptr_t phys, uintptr_t flags, bool overwrite) {
    if (virt & (PAGE_SIZE - 1)) return -1;
    if (phys & (PAGE_SIZE - 1)) return -1;

    uint64_t* pt = get_or_create_pt(pml4, virt);
    if (!pt) return -1;

    uint16_t idx = (virt >> 12) & 0x1FF;
    if (!overwrite && (pt[idx] & PT_PRESENT)) return -1;
    pt[idx] = (phys & ~FLAGS_MASK) | flags;
    return 0;
}

static int set_large_page(uint64_t* pml4, uintptr_t virt, uintptr_t phys, uintptr_t flags, bool overwrite) {
    if (virt & (LARGE_PAGE_SIZE - 1)) return -1;
    if (phys & (LARGE_PAGE_SIZE - 1)) return -1;

    uint64_t* pd = get_or_create_pd(pml4, virt);
    if (!pd) return -1;

    uint16_t idx = (virt >> 21) & 0x1FF;
    if (!overwrite && (pd[idx] & PT_PRESENT)) return -1;
    pd[idx] = (phys & ~LARGE_FLAGS_MASK) | flags | PT_PSE;
    return 0;
}

static size_t set_pages(uint64_t* pml4, uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
    if (virt & (PAGE_SIZE - 1)) return size;
    if (phys & (PAGE_SIZE - 1)) return size;
    if (size & (PAGE_SIZE - 1)) return size;

    size_t failed = 0;

    for (size_t i = 0; i < size; i += PAGE_SIZE) {
        int code = set_page(pml4, virt + i, phys + i, flags, overwrite);
        if (code) failed += PAGE_SIZE;
    }

    return failed;
}

static size_t set_large_pages(uint64_t* pml4, uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
    if (virt & (LARGE_PAGE_SIZE - 1)) return size;
    if (phys & (LARGE_PAGE_SIZE - 1)) return size;
    if (size & (LARGE_PAGE_SIZE - 1)) return size;

    size_t failed = 0;
    for (size_t i = 0; i < size; i += LARGE_PAGE_SIZE) {
        int code = set_large_page(pml4, virt + i, phys + i, flags, overwrite);
        if (code) failed += LARGE_PAGE_SIZE;
    }

    return failed;
}

static size_t set_range(uint64_t* pml4, uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags, bool overwrite) {
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

    if (pre_pad_size) failed += set_pages(pml4, virt, phys, pre_pad_size, flags, overwrite);
    if (large_size) failed += set_large_pages(pml4, large_virt, large_phys, large_size, flags, overwrite);
    if (pos_pad_size) failed += set_pages(pml4, pos_pad_virt, pos_pad_phys, pos_pad_size, flags, overwrite);

    return failed;
}

int vmm_map_hhdm() {
    if (state != VMM_IDENTITY) {
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

        failed +=set_range(kern_pml4, HHDM_BASE + entries[i].addr, entries[i].addr, entries[i].size, flags, 0);
        total += entries[i].size;
    }

    size_t vga_failed = set_range(kern_pml4, HHDM_BASE + 0xA0000, 0xA0000, 0x20000, PT_PRESENT | PT_WRITABLE | PT_GLOBAL | PT_NX | PT_PCD | PT_PWT, 0);
    if (vga_failed) printk(KERN_WARNING "[WARN] Failed to fully map VGA framebuffer\n");

    state = VMM_TRANSITION;

    if (failed) {
        printk(KERN_WARNING "[WARN] HHDM mapped %llu KiB and skipped %llu KiB\n", (total - failed) / 1024, failed / 1024);
        return -1;
    }
    
    printk("[OK] HHDM mapped %llu KiB\n", total / 1024);
    return 0;
}

int vmm_unmap_low_identity() {
    if (state != VMM_TRANSITION) {
        printk(KERN_ERR, "[ERR] VMM failed to unmap low identity\n\n");
        return -1;
    }
    kern_pml4[0] = 0;
    __asm__ volatile(
        "mov %%cr3, %%rax;"
        "mov %%rax, %%cr3"
        ::: "rax", "memory"
    ); // flush TLB
    state = VMM_VIRTUAL_ONLY;
    printk("[OK] VMM unmapped low identity\n");
    return 0;
}

size_t vmm_map(uint64_t* pml4, uintptr_t virt, uintptr_t phys, size_t size, uintptr_t flags) {
    if (pml4 == KERN_PML4) pml4 = kern_pml4;
    if (!pml4) return SIZE_MAX;
    if ((uintptr_t)pml4 % 0x1000) return SIZE_MAX;

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

        failed += set_pages(pml4, virt, phys, size, flags, 0);
        return failed;
    }
    return set_range(pml4, virt, phys, size, flags, 0);
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

int vmm_virt_range_has_mapping(uint64_t* pml4, uintptr_t virt, size_t size) {
    if (pml4 == KERN_PML4) pml4 = kern_pml4;
    if (!pml4) return -1;
    if ((uintptr_t)pml4 % 0x1000) return -1;

    uintptr_t end = virt + size;

    while (virt < end) {
        uint16_t idx = (virt >> 39) & 0x1FF;
        uint64_t entry = pml4[idx];
        if (!(entry & PT_PRESENT)) { 
            virt = ROUND_DOWN(virt, 0x8000000000) + 0x8000000000;
            continue; 
        }

        uint64_t* pdpt = phys_to_virt(entry & ~FLAGS_MASK);
        idx = (virt >> 30) & 0x1FF;
        entry = pdpt[idx];
        if (!(entry & PT_PRESENT)) {
            virt = ROUND_DOWN(virt, 0x40000000) + 0x40000000;
            continue;
        }
        if (entry & PT_PSE) return 1;

        uint64_t* pd = phys_to_virt(entry & ~FLAGS_MASK);
        idx = (virt >> 21) & 0x1FF;
        entry = pd[idx];
        if (!(entry & PT_PRESENT)) {
            virt = ROUND_DOWN(virt, LARGE_PAGE_SIZE) + LARGE_PAGE_SIZE;
            continue;
        }
        if (entry & PT_PSE) return 1;

        uint64_t* pt = phys_to_virt(entry & ~FLAGS_MASK);
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

int vmm_use_alloc_pages() {
    if (state < VMM_VIRTUAL_ONLY) return -1;
    state = VMM_READY;
    return 0;
}

uint64_t* vmm_create_user_pml4() {
    uint64_t* pml4 = __alloc_table();
    if (!pml4) return NULL;
    memset(pml4, 0, 4096);
    for (int i = 256; i < 512; i++)
        pml4[i] = kern_pml4[i];
    return pml4;
}

// TODO implement borrow mechanism
int vmm_destroy_user_pml4(uint64_t* pml4) {
    if (pml4 == kern_pml4) return -1;

    if (phys_to_virt(get_cr3() & CR3_ADDR_MASK) == pml4) {
        set_cr3(virt_to_phys(kern_pml4));
    }

    for (int i = 0; i < 256; i++) {
        uint64_t pml4_entry = pml4[i];
        if (!(pml4_entry & PT_PRESENT)) continue;

        uint64_t* pdpt = phys_to_virt(pml4_entry & (~FLAGS_MASK));
        for (int j = 0; j < 512; j++) {
            uint64_t pdpt_entry = pdpt[j];
            if (!(pdpt_entry & PT_PRESENT)) continue;
            
            uint64_t* pd = phys_to_virt(pdpt_entry & (~FLAGS_MASK));
            for (int k = 0; k < 512; k++) {
                uint64_t pd_entry = pd[k];
                if (!(pd_entry & PT_PRESENT)) continue;

                if (pd_entry & PT_PSE) {
                    void* addr = phys_to_virt(pd_entry & (~FLAGS_MASK));
                    if ((uintptr_t)addr % 0x200000) panic("VMM: corrupted 2MB page alignment detected at %p\n", addr);
                    struct page* page = addr_to_page(addr);
                    free_pages(page);
                    continue;
                }

                uint64_t* pt = phys_to_virt(pd_entry & (~FLAGS_MASK));
                for (int l = 0; l < 512; l++) {
                    uint64_t pt_entry = pt[l];
                    if (!(pt_entry & PT_PRESENT)) continue;
                    void* addr = phys_to_virt(pt_entry & (~FLAGS_MASK));
                    struct page* page = addr_to_page(addr);
                    free_pages(page);
                }

                struct page* pt_page = addr_to_page(pt);
                free_pages(pt_page);
            }

            struct page* pd_page = addr_to_page(pd);
            free_pages(pd_page);
        }

        struct page* pdpt_page = addr_to_page(pdpt);
        free_pages(pdpt_page);
    }

    struct page* pml4_page = addr_to_page(pml4);
    free_pages(pml4_page);

    return 0;
}

void vmm_set_table(uint64_t* pml4) {
    set_cr3(virt_to_phys(pml4));
}