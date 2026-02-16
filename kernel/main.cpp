#include <stdint.h>
#include <stdio.h>
#include "memory.h"
#include <hal/hal.h>
#include <boot/BootParams.h>
#include "logger.h"
#include "time.h"
#include <arch/i686/rtc.h>
#include <system/memory/page.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start(const BootParams* const bootParams) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    HAL_Initialize();
    PAGE_initialize(&bootParams->memory_info);

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

    printf("MEM -> total=%dB usable=%dB unused=%dB\n", PAGE_total_mem(), PAGE_usable_mem(), PAGE_unused_mem());

    PAGE_Block b1 = PAGE_alloc(2);
    PAGE_Block b2 = PAGE_alloc(3);
    PAGE_free(&b1);
    // PAGE_print(512);

    PAGE_alloc(3);
    // PAGE_print(512);

    printf("\n%d %d", PAGE_address(&b1), PAGE_page_count(&b1));
    printf("\n%d %d\n", PAGE_address(&b2), PAGE_page_count(&b2));

end:
    for (;;);
}