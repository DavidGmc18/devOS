#pragma once

void E_VGA_init();
void E_VGA_putc(char ch);
void E_VGA_write(const char* str, unsigned long n);
void E_VGA_log_write(int level, const char *str, unsigned long n);