#pragma once

int E_UART_init();
void E_UART_putc(char ch);
void E_UART_puts(const char* str);
void E_UART_write(const char* str, unsigned long n);
void E_UART_log_write(int level, const char *str, unsigned long n);