#pragma once

void early_vga_init();
void early_vga_putc(char ch);
void early_vga_write(const char* str, unsigned long n);
void early_vga_log_write(int level, const char *str, unsigned long n);