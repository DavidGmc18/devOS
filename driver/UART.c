#include "UART.h"
#include <arch/x86/io.h>

#define PORT 0x3F8

#define REG_DATA 0
#define REG_IER 1  // Interrupt Enable
#define REG_FCR 2  // FIFO Control
#define REG_LCR 3  // Line Control
#define REG_MCR 4  // Modem Control
#define REG_LSR 5  // Line Status

#define BSY 0x20

static int uart_present = 0;

int UART_init() {
    outb(PORT + REG_IER, 0x00); // Disable interrupts
    
    // Set Baud Rate
    outb(PORT + REG_LCR, 0x80); // Enable DLAB
    outb(PORT + 0, 0x03); // Divisor Low
    outb(PORT + 1, 0x00); // Divisor High
    
    // 8 bits, no parity, 1 stop bit
    outb(PORT + REG_LCR, 0x03);    
    
    // Enable FIFO, clear them
    outb(PORT + REG_FCR, 0xC7);    

    // Loopback test
    outb(PORT + REG_MCR, 0x1E);
    outb(PORT + REG_DATA, 0xAE);
    
    if (inb(PORT + REG_DATA) == 0xAE) {
        uart_present = 1;
        outb(PORT + REG_MCR, 0x0F); // Normal mode
    } else uart_present = 0;
    return !uart_present;
}

void UART_putc(char c) {
    if (!uart_present) return;
    while (!(inb(PORT + REG_LSR) & BSY)) iowait();
    outb(PORT + REG_DATA, c);
}