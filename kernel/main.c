#include <stdint.h>
#include <driver/UART.h>
#include <lib/printk.h>

void __attribute__((noreturn, section(".entry"))) entry() {
    UART_init();

    printk("Hello Kernel!\n");

    while (1) __asm__ volatile ("hlt" ::: "memory");
}