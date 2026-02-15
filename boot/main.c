#include <stdint.h>
#include <memory.h>
#include <driver/vga/vga_text.h>
#include <arch/i686/printk.h>
#include "FAT.h"
#include <driver/ata/ata.h>
#include "BootParams.h"

extern uint8_t __bss_start;
extern uint8_t __end;

uint8_t* kernel = (uint8_t*)0x104000;

typedef void (*KernelStart)(BootParams*);

void __attribute__((cdecl)) main(uint16_t boot_drive, uint8_t boot_partition, MemoryInfo* mem_info) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    // MBR
    uint8_t* buffer = (uint8_t*)0x300000;
    ATA_read28(boot_drive, 0, 1, buffer);
    uint32_t boot_partition_lba = *(uint32_t*)(buffer + 446 + boot_partition * 16 + 8);

    // Load kernel
    FAT_initialize(boot_drive, boot_partition_lba);
    FAT_load_file("KERNEL  BIN", kernel);

    BootParams boot_params = {
        .boot_device = {boot_drive, boot_partition},
        .memory_info = *mem_info
    };

    // Start kernel
    KernelStart kernelStart = (KernelStart)kernel;
    kernelStart(&boot_params);

    for (;;) ;
}