#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <boot/bootparams.h>
#include "logger.h"

#include <arch/i686/rtc.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start(BootParams* bootParams) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    HAL_Initialize();
    logf("MAIN", LOGGER_LVL_INFO, "Boot device: %x", bootParams->BootDevice);

    printf("Boot device: %x\n", bootParams->BootDevice);
    printf("Memory region count: %d\n", bootParams->Memory.RegionCount);
    uint64_t mem = 0;
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        printf("MEM: start=0x%llx length=0x%llx type=%x\n", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
        mem += bootParams->Memory.Regions[i].Length;
    }
    printf("%lluB\n", mem);

    printf("H\033[1;33mello world f\033[0mrom kernel!!!\n");
    printf("%d\n", -1234567);

    printf("%d:%d:%ds\n", rtc(RTC_HOURS), rtc(RTC_MINUTES), rtc(RTC_SECONDS));

end:
    for (;;);
}