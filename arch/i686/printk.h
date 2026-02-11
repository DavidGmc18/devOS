#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

void vprintk(const char* format, va_list args);

void printk(const char* format, ...);

#ifdef __cplusplus
}
#endif