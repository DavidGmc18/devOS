#include <driver/uart/early_uart.h>
#include <driver/vga/early_vga.h>
#include <printk.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/gdt.h>
#include <string.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;

void __attribute__((noreturn, section(".entry"))) entry() {
    cli();
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

    early_uart_init();
    early_vga_init();
    printk_sink_register((printk_sink_t){.name = "UART    ", .write = early_uart_log_write});
    printk_sink_register((printk_sink_t){.name = "VGA     ", .write = early_vga_log_write});
    printk("[OK] printk() active\n");

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    
    sti();
    printk("Kernel loaded at %#p\n", entry);

    while (1) halt();
}