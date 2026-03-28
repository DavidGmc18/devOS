#include "pic.h"
#include "idt.h"
#include <kernel/panic.h>
#include "isr.h"
#include "io.h"
#include "gdt.h"
#include <printk.h>
#include "tss.h"

void __attribute__((interrupt)) timer_irq(interrupt_frame_t* frame) {
    if (!frame) panic("Interrupt frame is NULL!\n");
    pic_send_EOI(0);
}

void __attribute__((interrupt)) keyboard_irq(interrupt_frame_t* frame) {
    if (!frame) panic("Interrupt frame is NULL!\n");
    uint8_t scancode = inb(0x60);
    pic_send_EOI(1);
}

#define IRQ_TO_INTERRUPT_OFFSET 32

void irq_set_gate(uint8_t irq, void* offset, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p) {
    p ? pic_unmask(irq) : pic_mask(irq);
    idt_set_gate(irq+IRQ_TO_INTERRUPT_OFFSET, offset, segment, ist, type, dpl, p);
}

void irq_init() {
    pic_remap(IRQ_TO_INTERRUPT_OFFSET, IRQ_TO_INTERRUPT_OFFSET+8);

    irq_set_gate(0, timer_irq, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);
    irq_set_gate(1, keyboard_irq, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);

    printk("[OK] IRQ initialized\n");
}