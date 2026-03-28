#include "isr.h"
#include "idt.h"
#include "io.h"
#include <kernel/panic.h>
#include "gdt.h"
#include "tss.h"
#include <printk.h>

void __attribute__((interrupt)) no_handler(InterruptFrame *frame) {
    if (!frame) panic("InterruptFrame is NULL!\n");
    panic("Unhandled exception!\n");
}

void __attribute__((interrupt)) no_handler_err(InterruptFrame *frame, uint64_t error_code) {
    if (!frame) panic("InterruptFrame is NULL!\n");
    panic("Unhandled exception!\n");
}

void ISR_init() {
    IDT_set_gate(0, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(1, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(2, no_handler, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    IDT_set_gate(3, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(4, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(5, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(6, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(7, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(8, no_handler_err, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    IDT_set_gate(9, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(10, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(11, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(12, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(13, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(14, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);
    IDT_set_gate(15, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(16, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(17, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(18, no_handler, GDT_KERNEL_CODE_SEGMENT, EMERG_IST, IDT_INTERRUPT_GATE, 0, 1);
    IDT_set_gate(19, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(20, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(21, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    for (int i = 22; i < 28; i++) IDT_set_gate(i, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(28, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(29, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(30, no_handler_err, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    IDT_set_gate(31, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_TRAP_GATE, 0, 1);
    for (int i = 32; i < 256; i++) IDT_set_gate(i, no_handler, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);

    printk("[OK] ISR initialized\n");
}