#pragma once

#include <stdarg.h>

void vfnprintk(void (*putc)(char), const char* format, va_list args);
void fnprintk(void (*putc)(char), const char* format, ...);

void printk_set_outfn(void (*putc)(char));
void vprintk(const char* format, va_list args);
void printk(const char* format, ...);