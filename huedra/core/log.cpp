#include "log.hpp"

#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace huedra {

void log(LogLevel level, const char* format, ...)
{
    const char* levelStr;
    switch (level)
    {
    case LogLevel::INFO:
        levelStr = "INFO";
        break;
    case LogLevel::WARNING:
        levelStr = "WARNING";
        break;
    case LogLevel::ERR:
        levelStr = "ERROR";
        break;
    default:
        levelStr = "UNKNOWN";
        break;
    }

    std::cout << "[" << levelStr << "] ";

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    std::cout << std::endl;

    if (level == LogLevel::ERR)
    {
        exit(1);
    }
}

} // namespace huedra