#include "pic.h"
#include "io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define INIT 0x11
#define MODE_8086 0x01
#define EOI 0x20

#define PIC2_IRQ 2

void PIC_remap(int offset1, int offset2) {
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

	outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void PIC_send_EOI(uint8_t irq) {
    outb(PIC1_COMMAND, EOI);
    if(irq >= 8)
		outb(PIC2_COMMAND, EOI);
}

void PIC_mask(uint8_t irq) {
    if (irq >= 8) {
        uint8_t mask = inb(PIC2_DATA);
        mask |= (1 << (irq-8));
        outb(PIC2_DATA, mask);
    } else {
        uint8_t mask = inb(PIC1_DATA);
        mask |= (1 << irq);
        outb(PIC1_DATA, mask);
    }
}

void PIC_unmask(uint8_t irq) {
    if (irq >= 8) {
        uint8_t mask = inb(PIC2_DATA);
        mask &= ~(1 << (irq-8));
        outb(PIC2_DATA, mask);
    } else {
        uint8_t mask = inb(PIC1_DATA);
        mask &= ~(1 << irq);
        outb(PIC1_DATA, mask);
    }
}