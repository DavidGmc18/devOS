#include "pic.h"
#include "idt.h"
#include <printk.h>
#include "isr.h"
#include "io.h"

void __attribute__((interrupt)) timer_irq(InterruptFrame *irf) {
    printk("Timer IRQ\n");
    PIC_send_EOI(0);
}

void __attribute__((interrupt)) keyboard_irq(InterruptFrame *irf) {
    uint8_t scancode = inb(0x60);
    printk("Keyboard IRQ: %#x\n", scancode);
    PIC_send_EOI(1);
}

#define KERNEL_SEGMENT 0x08

void IRQ_init() {
    PIC_remap(32, 40);
    PIC_unmask(2); // Enable PIC2

    // PIC_unmask(0);
    // IDT_set_gate(32, timer_irq, KERNEL_SEGMENT, 0, IDT_INTERRUPT_GATE, 0, 1);

    // PIC_unmask(1);
    // IDT_set_gate(33, keyboard_irq, KERNEL_SEGMENT, 0, IDT_INTERRUPT_GATE, 0, 1);

}