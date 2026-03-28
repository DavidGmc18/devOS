#include "early_uart.h"
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

int early_uart_init() {
    outb(PORT + REG_IER, 0x00); // Disable interrupts
    io_wait();
    
    // Set Baud Rate
    outb(PORT + REG_LCR, 0x80); // Enable DLAB
    io_wait();
    outb(PORT + 0, 0x03); // Divisor Low
    io_wait();
    outb(PORT + 1, 0x00); // Divisor High
    io_wait();
    
    // 8 bits, no parity, 1 stop bit
    outb(PORT + REG_LCR, 0x03);
    io_wait();
    
    // Enable FIFO, clear them
    outb(PORT + REG_FCR, 0xC7);
    io_wait();

    // Loopback test
    outb(PORT + REG_MCR, 0x1E);
    io_wait();
    outb(PORT + REG_DATA, 0xAE);
    io_wait();
    
    if (inb(PORT + REG_DATA) == 0xAE) {
        uart_present = 1;
        outb(PORT + REG_MCR, 0x0F); // Normal mode
        io_wait();
    } else uart_present = 0;
    return !uart_present;
}

void early_uart_putc(char ch) {
    if (!uart_present) return;
    while (!(inb(PORT + REG_LSR) & BSY)) io_wait();
    outb(PORT + REG_DATA, ch);
    io_wait();
}

void early_uart_puts(const char* str) {
    if (!uart_present) return;
    while (*str) {
        while (!(inb(PORT + REG_LSR) & BSY)) io_wait();
        outb(PORT + REG_DATA, *str++);
        io_wait();
    }
}

void early_uart_write(const char* str, unsigned long n) {
    if (!uart_present) return;
    for (unsigned long i = 0; i < n; i ++) {
        while (!(inb(PORT + REG_LSR) & BSY)) io_wait();
        outb(PORT + REG_DATA, str[i]);
        io_wait();
    }
}

static const char* get_level_ansi(int level) {
    switch (level) {
        case 0: return "\033[1;37;41m";
        case 1: return "\033[1;31m";
        case 2:
        case 3: return "\033[0;91m";
        case 4: return "\033[0;93m";
        case 5: return "\033[0;97m";
        default: return "\033[0m";
    }
}

void early_uart_log_write(int level, const char *str, unsigned long n) {
    if (!uart_present || !n) return;

    early_uart_puts(get_level_ansi(level));

    // Reset ANSI colors BEFORE the newline to prevent "color bleeding" into the 
    // next line; we write the first n-1 chars, then the Reset code, then the \n.
    early_uart_write(str, n-1);
    if (str[n-1] == '\n') {
        early_uart_puts("\033[0m\n");
    } else {
        early_uart_putc(str[n-1]);
        early_uart_puts("\033[0m");
    }
}