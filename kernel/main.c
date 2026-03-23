#include <stdint.h>
#include <driver/UART.h>

void __attribute__((noreturn, section(".entry"))) entry() {
    UART_init();

    while (1) __asm__ volatile ("hlt" ::: "memory");
}