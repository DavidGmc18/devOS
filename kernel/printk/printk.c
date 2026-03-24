#include <printk.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include "printf.h"

#define BUFFER_SIZE 256
static char buffer[BUFFER_SIZE];
static unsigned long buffer_fill;

#define DEFAULT_LEVEL 7
static int level;

#define MAX_SINKS 8
static printk_sink_t sinks[MAX_SINKS];

static void flush() {
    if (!buffer_fill) return;

    for (int i = 0; i < MAX_SINKS; i++) {
        if (sinks[i].name[0]) {
            sinks[i].write(level, buffer, buffer_fill);
        }
    }
    buffer_fill = 0;
}

void _putchar(char ch) {
    buffer[buffer_fill++] = ch;
    if (buffer_fill >= BUFFER_SIZE) {
        flush();
        buffer_fill = 0;
    }
}

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);

    buffer_fill = 0;
    if (format[0] == '\001' && format[1]) {
        level = format[1] - '0';
        format += 2;
    } else {
        level = DEFAULT_LEVEL;
    }

    vprintf_(format, args);

    flush();
    va_end(args);
}

printk_sink_t* printk_sink_register(printk_sink_t sink) {
    if (!sink.name[0])
        return NULL;

    for (int i = 0; i < MAX_SINKS; i++) {
        if (!sinks[i].name[0]) {
            sinks[i] = sink;
            return sinks+i;
        }
    }
    return NULL;
}

int printk_sink_unregister(printk_sink_t *sink) {
    if (sink < sinks || sink >= (sinks + MAX_SINKS))
        return -1; 
    sink->name[0] = 0;
    return 0;
}