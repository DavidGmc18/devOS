#include <stdint.h>
#include <stdio.h>
#include "memory.h"
#include <hal/hal.h>
#include <boot/BootParams.h>
#include <system/debug/logger.h>
#include "time.h"
#include <arch/i686/rtc.h>
#include <system/memory/PageAllocator.hpp>

extern uint8_t __bss_start;
extern uint8_t __end;

extern "C" void __attribute__((section(".entry"))) start(const BootParams* const bootParams) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    HAL_Initialize();
    // PAGE_initialize(&bootParams->memory_info);
    PageAllocator(&bootParams->memory_info);

    // logf("MAIN", LOGGER_LVL_INFO, "Boot device: %x", bootParams->BootDevice);

    // printf("Boot device: %x\n", bootParams->BootDevice);
    printf("Memory region count: %d\n", bootParams->memory_info.block_count);
    uint64_t mem = 0;
    for (int i = 0; i < bootParams->memory_info.block_count; i++) {
        printf("MEM: start=0x%llx length=0x%llx type=%x\n", 
            bootParams->memory_info.blocks[i].base,
            bootParams->memory_info.blocks[i].length,
            bootParams->memory_info.blocks[i].type);
        mem += bootParams->memory_info.blocks[i].length;
    }
    printf("%lluB\n", mem);

    printf("MEM:\n");
    printf("    Detected mem    %dB\n", PageAllocator::get_last_address());
    printf("    Usable mem      %dB\n", PageAllocator::get_usable_memory());
    printf("    Used mem        %dB\n", PageAllocator::get_used_memory());
    printf("    Free mem        %dB\n", PageAllocator::get_free_memory());


    {
        OwnedPageBlock block1 = PageAllocator::alloc(5);
        printf("Block1: 0x%x %dB\n", block1.address, block1.page_count * PageAllocator::PAGE_SIZE);

        OwnedPageBlock block2 = PageAllocator::alloc(4);
        printf("Block2: 0x%x %dB\n", block2.address, block2.page_count * PageAllocator::PAGE_SIZE);

        PageAllocator::free(block1);
        printf("Block1: 0x%x %dB\n", block1.address, block1.page_count * PageAllocator::PAGE_SIZE);

        OwnedPageBlock block3 = PageAllocator::alloc(6);
        printf("Block3: 0x%x %dB\n", block3.address, block3.page_count * PageAllocator::PAGE_SIZE);

        OwnedPageBlock block4 = PageAllocator::alloc(4);
        printf("Block4: 0x%x %dB\n", block4.address, block4.page_count * PageAllocator::PAGE_SIZE);

        OwnedPageBlock block5 = PageAllocator::alloc(1);
        printf("Block5: 0x%x %dB\n", block5.address, block5.page_count * PageAllocator::PAGE_SIZE);
    }

    printf("    Used mem        %dB\n", PageAllocator::get_used_memory());

    OwnedPageBlock block6 = PageAllocator::alloc(16);
    printf("Block6: 0x%x %dB\n", block6.address, block6.page_count * PageAllocator::PAGE_SIZE);

    block6.alloc(1);

end:
    for (;;);
}