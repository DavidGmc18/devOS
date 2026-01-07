#pragma once

#include "isr.h"

typedef void (*IRQ_Handler)(Registers* regs);

void i686_IRQ_Initialize();
void i686_IRQ_RegisterHandler(int irq, IRQ_Handler handler);