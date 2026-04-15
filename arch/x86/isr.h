#pragma once

#include <stdint.h>

typedef struct {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} interrupt_frame_t;

void isr_init();
void isr_set_gate(uint8_t interrupt, uint16_t segment,  uint8_t ist, uint8_t type, uint8_t dpl, bool p);