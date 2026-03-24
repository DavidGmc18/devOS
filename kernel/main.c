#include <stdint.h>
#include <driver/UART/UART.h>
#include <driver/VGA/VGA.h>
#include <lib/printk.h>

void __attribute__((noreturn, section(".entry"))) entry() {
    UART_init();
    VGA_init();

    printk_sink_t sink_uart = {
        .name = "UART    ",
        .write = UART_log_write
    };
    printk_sink_register(sink_uart);
    printk_sink_t sink_vga = {
        .name = "VGA     ",
        .write = VGA_log_write
    };
    printk_sink_register(sink_vga);

    printk(KERN_DEBUG "DEBUG\n");
    printk(KERN_INFO "INFO\n");
    printk(KERN_NOTICE "NOTICE\n");
    printk(KERN_WARNING "WARNING\n");
    printk(KERN_ERR "ERR\n");
    printk(KERN_CRIT "CRIT\n");
    printk(KERN_ALERT "ALERT\n");
    printk(KERN_EMERG "EMERG\n");
    printk("TEST\n");

    while (1) __asm__ volatile ("hlt" ::: "memory");
}