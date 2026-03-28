#include "pic.h"
#include "io.h"
#include <kernel/panic.h>
#include <printk.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define INIT 0x11
#define MODE_8086 0x01
#define EOI 0x20

#define PIC2_IRQ 2

void PIC_remap(int offset1, int offset2) {
    if (offset1 < 32 || offset1 >= 248) panic("Attemoted to remap PIC1 to IRQ %#X!\n", offset1);
    if (offset2 < 32 || offset2 >= 248) panic("Attemoted to remap PIC2 to IRQ %#X!\n", offset2);
    int diff = (offset1 > offset2) ? (offset1 - offset2) : (offset2 - offset1);
    if (diff < 8) panic("Requested remap would cause PIC2 to overlap PIC1!\n", offset1, offset2);

	outb(PIC1_COMMAND, INIT);
	io_wait();
	outb(PIC2_COMMAND, INIT);
	io_wait();

	outb(PIC1_DATA, offset1);
	io_wait();
	outb(PIC2_DATA, offset2);
	io_wait();

    outb(PIC1_DATA, 1 << PIC2_IRQ);
    io_wait();
    outb(PIC2_DATA, PIC2_IRQ);
    io_wait();

    outb(PIC1_DATA, MODE_8086);
    io_wait();
    outb(PIC2_DATA, MODE_8086);
    io_wait();

    // Mask all IRQs but only enable IRQ2 which is PIC2
	outb(PIC1_DATA, 0xFF - (1 << PIC2_IRQ));
    io_wait();
    outb(PIC2_DATA, 0xFF);
    io_wait();

    printk("[OK] PIC remapped\n");
}

void PIC_send_EOI(uint8_t irq) {
    outb(PIC1_COMMAND, EOI);
    io_wait();
    if(irq >= 8) {
		outb(PIC2_COMMAND, EOI);
        io_wait();
    }
}

void PIC_mask(uint8_t irq) {
    if (irq >= 8) {
        uint8_t mask = inb(PIC2_DATA);
        mask |= (1 << (irq-8));
        outb(PIC2_DATA, mask);
        io_wait();
    } else {
        uint8_t mask = inb(PIC1_DATA);
        mask |= (1 << irq);
        outb(PIC1_DATA, mask);
        io_wait();
    }
}

void PIC_unmask(uint8_t irq) {
    if (irq >= 8) {
        uint8_t mask = inb(PIC2_DATA);
        mask &= ~(1 << (irq-8));
        outb(PIC2_DATA, mask);
        io_wait();
    } else {
        uint8_t mask = inb(PIC1_DATA);
        mask &= ~(1 << irq);
        outb(PIC1_DATA, mask);
        io_wait();
    }
}