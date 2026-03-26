#pragma once

#include <stdint.h>

void PIC_remap(int offset1, int offset2);
void PIC_send_EOI(uint8_t irq);
void PIC_mask(uint8_t irq);
void PIC_unmask(uint8_t irq);