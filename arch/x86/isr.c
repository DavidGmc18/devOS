#include "isr.h"
#include "idt.h"
#include "io.h"
#include <panic.h>
#include "gdt.h"
#include "tss.h"
#include <printk.h>
#include "interrupt.h"
#include "irq.h"

extern uint64_t isr_table[256];

void isr_set_gate(uint8_t interrupt, uint16_t segment,  uint8_t ist, uint8_t type, uint8_t dpl, bool p) {
    if (interrupt >= 256) panic("Invalid vector ID\n");
    idt_set_gate(interrupt, (void*)isr_table[interrupt], segment, ist, type, dpl, p);
}

void isr_init() {
    isr_set_gate(0, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(1, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(2, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    isr_set_gate(3, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(4, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(5, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(6, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(7, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(8, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    isr_set_gate(9, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(10, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(11, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(12, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(13, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(14, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);
    isr_set_gate(15, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(16, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(17, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(18, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    isr_set_gate(19, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(20, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(21, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    for (int i = 22; i < 28; i++)
        isr_set_gate(i, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(28, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(29, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(30, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    isr_set_gate(31, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    for (int i = 32; i < 256; i++)
        isr_set_gate(i, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);

    printk("[OK] ISR initialized\n");
}

void __isr_dispatch(struct regs *r) {
    if (r->vector_id < 32) {
        panic("Unhandled exception with vector ID %d\n", r->vector_id);
    } else if (r->vector_id < 256) {
        irq_dispatch(r);
    } else {
        panic("IST dispatch received invalid vector ID %d\n", r->vector_id);
    }
}