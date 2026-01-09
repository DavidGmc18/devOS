#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/irq_handlers.h>
#include <arch/i686/vga_text.h>

void HAL_Initialize() {
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    i686_IRQ_Handlers();

    VGA_scr(80, 25, (uint8_t*)0xB8000);
}