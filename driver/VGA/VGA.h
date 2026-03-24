#pragma once

void VGA_init();
void VGA_putc(char ch);
void VGA_write(const char* str, unsigned long n);
void VGA_log_write(int level, const char *str, unsigned long n);