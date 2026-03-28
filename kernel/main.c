#include <stdint.h>
#include <driver/UART/E_UART.h>
#include <driver/VGA/E_VGA.h>
#include <printk.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/gdt.h>

void __attribute__((noreturn, section(".entry"))) entry() {
    cli();

    E_UART_init();
    E_VGA_init();
    printk_sink_register((printk_sink_t){.name = "UART    ", .write = E_UART_log_write});
    printk_sink_register((printk_sink_t){.name = "VGA     ", .write = E_VGA_log_write});
    printk("[OK] Printk active\n");

    GDT_init();
    IDT_init();
    ISR_init();
    IRQ_init();
    sti();

    while (1) halt();
}