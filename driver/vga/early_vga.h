#pragma once

void early_vga_init(unsigned char* vbuf);
void early_vga_clrscr();
void early_vga_putc(char ch);
void early_vga_write(const char* str, unsigned long n);
void early_vga_log_write(int level, const char *str, unsigned long n);