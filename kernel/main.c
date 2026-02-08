#include <stdint.h>
#include <stdio.h>
#include "memory.h"
#include <hal/hal.h>
#include <boot/bootparams.h>
#include "logger.h"
#include "time.h"

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

    printf("%d:%d:%ds\n", i686_RTC_get(RTC_HOUR), i686_RTC_get(RTC_MIN), i686_RTC_get(RTC_SEC));

    struct tm time;
    time_tm(&time);
    char asc[26] = "                    ";
    asctime(asc, 1, &time);
    printf("%s\n", asc);
    asctime(asc, 26, &time);
    printf("%s", asc);

    char strf[96];
    strftime(strf, 96, "%Y:%y:%b%h%m:%j:%d:%a:%w:%Hh:%Ih:%Mm:%Ss--%x", &time);
    printf("%s\n\n\n\n", strf);

    char strf2[32];

    for (int i = 0; i < 32; i++) {
        strf2[i] = 'X';
    }

    printf("%s", asc);
    strftime(strf2, 26, "%x\n", &time);
    printf("%s", strf2);

end:
    for (;;);
}