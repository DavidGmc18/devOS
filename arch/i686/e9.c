#include "e9.h"
#include "i686.h"

void e9_putc(char ch) {
    i686_outb(0xE9, ch);
}

void e9_putn(const char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        e9_putc(str[i]);
    }
}