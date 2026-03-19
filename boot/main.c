#include <bl/boot.h>
#include <string.h>
#include "fat.h"

extern uint8_t __bss_start;
extern uint8_t __bss_end;

void __attribute__((cdecl, noreturn, section(".entry"))) entry(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

    // 1. Load kernel.bin into mem -> FAT
    // 2. Enter Long-mode -> Research
    // 3. Jump to kernel

    FAT_init(boot_services->disk_read);
    FAT_dev dev;
    FAT_dev_init(&dev, &boot_info->disk);
    boot_services->printk("lba=%d sectors=%d fat=%d root_dir=%d data=%d sectors_per_cluster=%d\n", dev.lba, dev.sectors, dev.fat, dev.root_dir_cluster, dev.data, dev.sectors_per_cluster);

    FAT_read(&dev, "system/kernel.bin", (void*)0x100000);
    boot_services->printk((char*)0x100000);

    while (1) __asm__ volatile ("hlt" ::: "memory");
}