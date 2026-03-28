#include <stdint.h>
#include <driver/UART/UART.h>
#include <driver/VGA/VGA.h>
#include <printk.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/gdt.h>

void __attribute__((noreturn, section(".entry"))) entry() {
    cli();
    GDT_init();
    IDT_init();
    ISR_init();
    IRQ_init();
    sti();

    UART_init();
    VGA_init();

    printk_sink_t sink_uart = {
        .name = "UART    ",
        .write = UART_log_write
    };
    printk_sink_t sink_vga = {
        .name = "VGA     ",
        .write = VGA_log_write
    };

    // Forward printk to serial and VGA
    printk_sink_register(sink_uart);
    printk_sink_register(sink_vga);

    while (1) halt();
}