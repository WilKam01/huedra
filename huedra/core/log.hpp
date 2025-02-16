#pragma once

#include <iostream>

namespace huedra {

enum class LogLevel
{
    D_INFO, // Debug info
    INFO,
    WARNING,
    ERR
};

void log(LogLevel level, const char* formatStr, auto&&... args)
{
    const char* levelStr;
    switch (level)
    {
    case LogLevel::D_INFO:
        levelStr = "DEBUG INFO";
        break;
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

    try
    {
        std::string formattedMessage = std::vformat(formatStr, std::make_format_args(args...));
        std::cout << "[" << levelStr << "] " << formattedMessage << "\n";
    }
    catch (const std::format_error& error)
    {
        std::cout << "log(): " << error.what() << "\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "log(): Unexpected error: " << error.what() << "\n";
    }

    if (level == LogLevel::ERR)
    {
        exit(1);
    }
}

} // namespace huedra