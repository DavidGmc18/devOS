#include <stdint.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void* memset(void* ptr, int value, uint16_t num) {
    uint8_t* u8Ptr = (uint8_t*)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

void __attribute__((section(".entry"))) start(uint16_t drive, uint8_t partition) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

for(;;);

}