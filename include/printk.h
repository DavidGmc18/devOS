#pragma once

#include <stdarg.h>

#define KERN_EMERG   "\001" "0"
#define KERN_ALERT   "\001" "1"
#define KERN_CRIT    "\001" "2"
#define KERN_ERR     "\001" "3"
#define KERN_WARNING "\001" "4"
#define KERN_NOTICE  "\001" "5"
#define KERN_INFO    "\001" "6"
#define KERN_DEBUG   "\001" "7"

int vprintkl(int level, const char* format, va_list args);
int vprintk(const char* format, va_list args);

int printkl(int level, const char* format, ...);
int printk(const char* format, ...);

typedef struct {
    char name[8]; // name[0] = 0 -> sink not present; name[0] != 0 => sink present
    void (*write)(int level, const char *str, unsigned long n);
} printk_sink_t;

printk_sink_t* printk_sink_register(printk_sink_t sink);
int printk_sink_unregister(printk_sink_t *sink);