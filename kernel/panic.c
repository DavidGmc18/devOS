#include "panic.h"
#include <printk.h>
#include <arch/x86/io.h>

void __attribute__((noreturn)) vpanic(const char* format, va_list args) {
    printkl(0, "KERNEL PANIC: ");
    vprintkl(0, format, args);
    cli();
    while (1) halt();
    __builtin_unreachable();
}

void __attribute__((noreturn)) panic(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vpanic(format, args);
    va_end(args);
}