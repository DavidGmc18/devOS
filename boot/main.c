#include <bl/boot.h>
#include <string.h>
#include "fat.h"

// TODO test FAT

extern uint8_t __bss_start;
extern uint8_t __bss_end;

#define PT_PRESENT 1
#define PT_WRITABLE 2
#define PAGE_PSE (1 << 7)
__attribute__((aligned(4096))) uint64_t pml4[512];
__attribute__((aligned(4096))) uint64_t pdpt_l[512];
__attribute__((aligned(4096))) uint64_t pdpt_h[512];
__attribute__((aligned(4096))) uint64_t pd_l[512];
__attribute__((aligned(4096))) uint64_t pd_h[512];

extern void kernel64_entry(void* pml4_addr, uint64_t kernel_addr);

void __attribute__((cdecl, noreturn, section(".entry"))) entry(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

// Load kernel
    fat_init(boot_services->disk_read);
    fat_dev_t dev;
    fat_dev_init(&dev, &boot_info->disk);
    void* kernel_addr = (void*)0x200000;
    int err = fat_read(&dev, "system/kernel.bin", kernel_addr);
    boot_services->printk("FAT_READ_ERR=%d\n", err);

// Enter Long-mode & jump to kernel
    // 0x000000 - 0x200000 => 0x000000 - 0x200000
    pml4[0] = (uint64_t)(uintptr_t)pdpt_l | PT_PRESENT | PT_WRITABLE;
    pdpt_l[0] = (uint64_t)(uintptr_t)pd_l | PT_PRESENT | PT_WRITABLE;
    pd_l[0] = (uint64_t)0x0 | PT_PRESENT | PT_WRITABLE | PAGE_PSE;

    // 0x200000 - 0x400000 => 0xFFFFFFFF80000000 - 0xFFFFFFFF82000000
    pml4[511] = (uint64_t)(uintptr_t)pdpt_h | PT_PRESENT | PT_WRITABLE;
    pdpt_h[510] = (uint64_t)(uintptr_t)pd_h | PT_PRESENT | PT_WRITABLE;
    pd_h[0] = (uint64_t)(uintptr_t)kernel_addr | PT_PRESENT | PT_WRITABLE | PAGE_PSE;

    kernel64_entry(pml4, 0xFFFFFFFF80000000);

    while (1) __asm__ volatile ("hlt" ::: "memory");
}