#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IDT_INTERRUPT_GATE 0x0E
#define IDT_TRAP_GATE 0x0F

void IDT_init();
void IDT_set_gate(int interrupt, void* offset, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p);