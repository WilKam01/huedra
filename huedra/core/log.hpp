#pragma once

namespace huedra {

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};

void log(LogLevel level, const char* format, ...);

} // namespace huedra