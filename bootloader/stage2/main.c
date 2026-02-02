#include <stdint.h>
#include "fat.h"
#include "memdefs.h"
#include <memory.h>
#include <boot/bootparams.h>
#include "memdetect.h"
#include "ata.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/printk.h>

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams g_bootParams;

typedef void (*KernelStart)(BootParams* bootParams);

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    // TODO we naively assume boot dirve is ATA:0 disk

    if (!FAT_Initialize(0))
    {
        printk("FAT init error\r\n");
        goto end;
    }

    // prepare boot params
    g_bootParams.BootDevice = bootDrive;
    Memory_Detect(&g_bootParams.Memory); 

    // load kernel
    FAT_File* fd = FAT_Open(0, "/kernel.bin");
    uint32_t read;
    uint8_t* kernelBuffer = Kernel;
    while ((read = FAT_Read(0, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer)))
    {
        memcpy(kernelBuffer, KernelLoadBuffer, read);
        kernelBuffer += read;
    }
    FAT_Close(fd);

    // execute kernel
    KernelStart kernelStart = (KernelStart)Kernel;
    kernelStart(&g_bootParams);

end:
    for (;;);
}