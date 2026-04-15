#include "irq.h"
#include "pic.h"
#include "idt.h"
#include <panic.h>
#include "isr.h"
#include "io.h"
#include "gdt.h"
#include <printk.h>
#include "tss.h"

#define IRQ_TO_INTERRUPT_OFFSET 32

void irq_set_gate(uint8_t irq, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p) {
    p ? pic_unmask(irq) : pic_mask(irq);
    isr_set_gate(irq+IRQ_TO_INTERRUPT_OFFSET, segment, ist, type, dpl, p);
}

void irq_init() {
    pic_remap(IRQ_TO_INTERRUPT_OFFSET, IRQ_TO_INTERRUPT_OFFSET+8);

    irq_set_gate(0, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);
    irq_set_gate(1, GDT_KERNEL_CODE_SEGMENT, NO_IST, IDT_INTERRUPT_GATE, 0, 1);

    printk("[OK] IRQ initialized\n");
}

void irq_dispatch(struct regs* r) {
    // printk(KERN_ERR "[ERR] Unhandled IRQ with vector ID %d\n", r->vector_id);
    pic_send_EOI(r->vector_id - IRQ_TO_INTERRUPT_OFFSET);
}