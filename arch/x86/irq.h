#pragma once

void IRQ_init();
void IRQ_set_gate(uint8_t irq, void* offset, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p);