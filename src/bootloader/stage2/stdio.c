#include "stdio.h"
#include "x86.h"

void putc(char c) {
    x86_Video_WriteCharReletype(c, 0);
}

void puts(const char* str) {
    while (*str) {
        x86_Video_WriteCharReletype(*str, 0);
        str++;
    }
}