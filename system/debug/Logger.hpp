#pragma once

#include "logger.h"

class Logger {
    const char* module;

public:
    Logger(const char* module);

    void log(LoggerLevel level, const char* format, ...);
};