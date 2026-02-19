#include "Logger.hpp"

#include <stdarg.h>

Logger::Logger(const char* module)
    : module(module)
{}

void Logger::log(LoggerLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logf(module, level, format, args);
    va_end(args);
}