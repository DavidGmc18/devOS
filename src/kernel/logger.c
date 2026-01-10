#include "logger.h"
#include <stdio.h>

static const char* g_LogSeverityColors[] = {
    [LOGGER_LVL_DEBUG]  = "\033[2;37m",
    [LOGGER_LVL_INFO]   = "\033[37m",
    [LOGGER_LVL_WARN]   = "\033[1;33m",
    [LOGGER_LVL_ERROR]  = "\033[1;31m",
    [LOGGER_LVL_FATAL]  = "\033[1;37;41m",
};

static size_t g_LogSeverityColorsLen[] = {
    [LOGGER_LVL_DEBUG]  = sizeof("\033[2;37m"),
    [LOGGER_LVL_INFO]   = sizeof("\033[37m"),
    [LOGGER_LVL_WARN]   = sizeof("\033[1;33m"),
    [LOGGER_LVL_ERROR]  = sizeof("\033[1;31m"),
    [LOGGER_LVL_FATAL]  = sizeof("\033[1;37;41m"),
};

static const char g_ColorReset[] = "\033[0m";

void logf(const char* module, LoggerLevel level, const char* format, ...) {
    if (level < MIN_LOG_LEVEL)
        return;

    va_list args;
    va_start(args, format);

    fd_t stream = level > LOGGER_LVL_INFO ? VFS_FD_STDERR : VFS_FD_DEBUG;

    fputn(g_LogSeverityColors[level], stream, g_LogSeverityColorsLen[level]);
    fprintf(stream, "[%s] ", module);
    vfprintf(stream, format, args);
    fputn(g_ColorReset, stream, sizeof(g_ColorReset));
    fputc('\n', stream);

    va_end(args);
}