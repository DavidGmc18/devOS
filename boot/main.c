#include <bl/boot.h>
#include <string.h>
#include "fat.h"
#include "vmm.h"
#include <arch/x86/e820.h>

#define KERN_PHYS 0x200000
#define KERN_VIRT 0xFFFFFFFF80000000
static const char* KERNEL_FILE = "system/kernel.bin";

extern uint8_t __bss_start;
extern uint8_t __bss_end;

extern void kernel64_entry(void* pml4_addr, uint64_t kernel_virt, struct e820_table* e820_table_ptr);

static struct e820_entry e820_entries[256];
static struct e820_table e820_table;

void __attribute__((cdecl, noreturn, section(".entry"))) entry(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

    fat_dev_t dev;
    fat_init(boot_services->disk_read);
    if (fat_dev_init(&dev, &boot_info->disk)) {
        boot_services->printk("Falied to initialize FAT for boot volume\n");
        goto end;
    }

    if (fat_read(&dev, KERNEL_FILE, (void*)KERN_PHYS)) {
        boot_services->printk("Falied to read '%s'\n", KERNEL_FILE);
        goto end;
    }

    if (vmm_init(KERN_PHYS, KERN_VIRT)) {
        boot_services->printk("Falied to initialize VMM\n");
        goto end;
    }

    e820_table.entries_count = boot_info->memory_info.block_count;
    e820_table.entries = e820_entries;
    for (int i = 0; i < boot_info->memory_info.block_count; i++) {
        BL_MemoryBlock* block = boot_info->memory_info.blocks+i;
        e820_entries[i].addr = block->base;
        e820_entries[i].size = block->length;
        e820_entries[i].type = block->type;
    }

    kernel64_entry(vmm_get_pml4(), KERN_VIRT, &e820_table);

end:
    boot_services->printk("Power-off your computer");
    while (1) __asm__ volatile ("hlt" ::: "memory");
    __builtin_unreachable();
}