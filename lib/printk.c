#include "printk.h"
#include <driver/UART.h>

static void fnputs(void (*putc)(char), const char* str) {
    while (*str) {
        putc(*str++);
    }
}

static char digits[16] = "0123456789ABCDEF";

static void fnprint_unsigned(void (*putc)(char), unsigned int num, unsigned int radix) {
    if (radix < 2 || radix > 16) {
        fnputs(putc, "BAD_RADIX");
        return;
    }

    if (num == 0) {
        putc('0');
        return;
    }

    char buffer[32];
    int i = 0;
    while (num > 0) {
        buffer[i++] = digits[num % radix];
        num /= radix;
    }

    while (i-- > 0) {
        putc(buffer[i]);
    }
}

static void fnprint_signed(void (*putc)(char), int num, unsigned int radix) {
    if (num < 0) {
        putc('-');
        num = -num;
    }
    fnprint_unsigned(putc, (int)num, radix);
}

void vfnprintk(void (*putc)(char), const char* format, va_list args) {
    int specifier = 0;

    while (*format) {
        if (!specifier) {
            switch (*format) {
                case '%':
                    specifier = 1;
                    break;

                default:
                    putc(*format);
                    break;
            }
        } else {
            switch (*format) {
                case 's':
                    fnputs(putc, va_arg(args, const char*));
                    break;

                case 'c':
                    putc((char)va_arg(args, int));
                    break;

                case 'd':
                case 'i':
                    fnprint_signed(putc, va_arg(args, int), 10);
                    break;

                case 'u':
                    fnprint_unsigned(putc, va_arg(args, unsigned int), 10);
                    break;

                case 'x':
                case 'X':
                    fnprint_unsigned(putc, va_arg(args, unsigned int), 16);
                    break;    
            
                default:
                    putc('%');
                    putc(*format);
                    break;
            }
            specifier = 0;
        }
        format++;
    }
}

void fnprintk(void (*putc)(char), const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfnprintk(putc, format, args);
    va_end(args);
}

static void (*outfn)(char) = UART_putc;

void printk_set_outfn(void (*putc)(char)) {
    outfn = putc;
}

void vprintk(const char* format, va_list args) {
    vfnprintk(outfn, format, args);
}

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfnprintk(outfn, format, args);
    va_end(args);
}