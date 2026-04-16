#include "pit.h"

#include <stdint.h>
#include "io.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

void pit_set_freq(unsigned long hz) {
    uint16_t divisor = 1193182 / hz;
    outb(PIT_COMMAND, 0x34);
    io_wait();
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)(divisor >> 8));
    io_wait();
}