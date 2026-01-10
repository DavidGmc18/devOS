#pragma once

#define MIN_LOG_LEVEL LOGGER_LVL_INFO

typedef enum {
    LOGGER_LVL_DEBUG = 0,
    LOGGER_LVL_INFO = 1,
    LOGGER_LVL_WARN = 2,
    LOGGER_LVL_ERROR = 3,
    LOGGER_LVL_FATAL = 4
} LoggerLevel;

void logf(const char* module, LoggerLevel level, const char* format, ...);