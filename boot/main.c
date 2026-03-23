#include <bl/boot.h>
#include <string.h>
#include "fat.h"

// TODO test FAT

extern uint8_t __bss_start;
extern uint8_t __bss_end;

#define PT_PRESENT 1
#define PT_WRITABLE 2
__attribute__((aligned(4096))) uint64_t pml4[512];
__attribute__((aligned(4096))) uint64_t pdpt[512];
__attribute__((aligned(4096))) uint64_t pd[512];
__attribute__((aligned(4096))) uint64_t pt[512];

extern void kernel64_entry(void* pml4_addr, void* kernel_addr);

void __attribute__((cdecl, noreturn, section(".entry"))) entry(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

// Load kernel
    FAT_init(boot_services->disk_read);
    FAT_dev dev;
    FAT_dev_init(&dev, &boot_info->disk);
    void* kernel_addr = (void*)0x100000;
    int err = FAT_read(&dev, "system/kernel.bin", kernel_addr);
    boot_services->printk("FAT_READ_ERR=%d\n", err);

// Enter Long-mode & jump to kernel
    memset(pml4, 0, 4096);
    memset(pdpt, 0, 4096);
    memset(pd, 0, 4096);
    memset(pt, 0, 4096);
    pml4[0] = (uint64_t)(uintptr_t)pdpt | PT_PRESENT | PT_WRITABLE;
    pdpt[0] = (uint64_t)(uintptr_t)pd | PT_PRESENT | PT_WRITABLE;
    pd[0] = (uint64_t)(uintptr_t)pt | PT_PRESENT | PT_WRITABLE;
    for (int i = 0; i < 512; i++)
        pt[i] = (uint64_t)(i * 0x1000) | PT_PRESENT | PT_WRITABLE;

    kernel64_entry(pml4, kernel_addr);

    while (1) __asm__ volatile ("hlt" ::: "memory");
}