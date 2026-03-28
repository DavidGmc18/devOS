#pragma once

int early_uart_init();
void early_uart_putc(char ch);
void early_uart_puts(const char* str);
void early_uart_write(const char* str, unsigned long n);
void early_uart_log_write(int level, const char *str, unsigned long n);