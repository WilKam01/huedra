#pragma once

namespace huedra {

enum class LogLevel
{
    INFO,
    WARNING,
    ERR
};

void log(LogLevel level, const char* format, ...);

} // namespace huedra