#include "pic.h"
#include "idt.h"
#include <kernel/panic.h>
#include "isr.h"
#include "io.h"
#include "gdt.h"
#include <printk.h>

void __attribute__((interrupt)) timer_irq(InterruptFrame* frame) {
    if (!frame) panic("InterruptFrame* is NULL!\n");
    PIC_send_EOI(0);
}

void __attribute__((interrupt)) keyboard_irq(InterruptFrame* frame) {
    if (!frame) panic("InterruptFrame* is NULL!\n");
    uint8_t scancode = inb(0x60);
    PIC_send_EOI(1);
}

#define IRQ_TO_INTERRUPT_OFFSET 32

void IRQ_set_gate(uint8_t irq, void* offset, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p) {
    p ? PIC_unmask(irq) : PIC_mask(irq);
    IDT_set_gate(irq+IRQ_TO_INTERRUPT_OFFSET, offset, segment, ist, type, dpl, p);
}

void IRQ_init() {
    PIC_remap(IRQ_TO_INTERRUPT_OFFSET, IRQ_TO_INTERRUPT_OFFSET+8);

    IRQ_set_gate(0, timer_irq, GDT_KERNEL_CODE_SEGMENT, 0, IDT_INTERRUPT_GATE, 0, 1);
    IRQ_set_gate(1, keyboard_irq, GDT_KERNEL_CODE_SEGMENT, 0, IDT_INTERRUPT_GATE, 0, 1);

    printk("[OK] IRQ initialized\n");
}