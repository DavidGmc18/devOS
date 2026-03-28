#include "printk.h"
#include <stddef.h>
#include <stdint.h>
#include "printf.h"
#include <arch/x86/io.h>

#define BUFFER_SIZE 256
#define DEFAULT_LEVEL 7
typedef struct {
    char buffer[BUFFER_SIZE];
    size_t buffer_fill;
    int level;
} printk_buffer_t;

#define MAX_SINKS 8
static printk_sink_t sinks[MAX_SINKS];

static void flush(printk_buffer_t* buffer) {
    if (!buffer) return;
    if (!buffer->buffer_fill) return;

    for (int i = 0; i < MAX_SINKS; i++) {
        if (sinks[i].name[0]) {
            sinks[i].write(buffer->level, buffer->buffer, buffer->buffer_fill);
        }
    }
    buffer->buffer_fill = 0;
}

static void putc(char ch, void* arg) {
    if (!arg) return;
    printk_buffer_t* buffer = (printk_buffer_t*)arg;

    if (buffer->buffer_fill >= BUFFER_SIZE)
        flush(buffer);

    buffer->buffer[buffer->buffer_fill++] = ch;
}

void vprintkl(int level, const char* format, va_list args) {
    if (!format) return;

    rflags_t rflags = get_rflags();
    cli();

    printk_buffer_t buffer = {
        .buffer_fill=0,
        .level=level
    };

    vfctprintf(putc, &buffer, format, args);
    flush(&buffer);

    if (rflags.intf) sti();
}

void vprintk(const char* format, va_list args) {
    if (!format) return;

    int level = DEFAULT_LEVEL;
    if (format[0] == '\001' && format[1]) {
        level = format[1] - '0';
        format += 2;
    }

    vprintkl(level, format, args);
}

void printkl(int p_level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintkl(p_level, format, args);
    va_end(args);
}

void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintk(format, args);
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