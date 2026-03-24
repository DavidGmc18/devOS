#pragma once

int UART_init();
void UART_putc(char ch);
void UART_puts(const char* str);
void UART_write(const char* str, unsigned long n);
void UART_log_write(int level, const char *str, unsigned long n);