#pragma once

#include <format>
#include <iostream>
#include <source_location>

namespace huedra::log {

enum class Level
{
    DEBUG_,
    INFO,
    WARNING,
    ERROR,
    FATAL,
};

enum class Type
{
    NORMAL,
    LOCATION,
    FUNCTION
};

template <typename... Args>
struct internal
{
    internal(Level level, Type type, std::string_view fmt, std::source_location loc, Args&&... args)
    {
#ifndef DEBUG
        if (level == Level::DEBUG_)
        {
            return;
        }
#endif

        std::string_view levelStr = "UNKNOWN";
        switch (level)
        {
        case Level::DEBUG_:
            levelStr = "DEBUG";
            break;
        case Level::INFO:
            levelStr = "INFO";
            break;
        case Level::WARNING:
            levelStr = "WARNING";
            break;
        case Level::ERROR:
            levelStr = "ERROR";
            break;
        case Level::FATAL:
            levelStr = "FATAL";
            break;
        }

        try
        {
            std::string prepend;
            switch (type)
            {
            case Type::LOCATION:
                prepend = std::format("[{}] {}:{}:{}: ", levelStr, loc.file_name(), loc.line(), loc.column());
                break;
            case Type::FUNCTION:
                prepend = std::format("[{}] {}: ", levelStr, loc.function_name());
                break;
            default:
                prepend = std::format("[{}] ", levelStr);
                break;
            }

            std::string message = std::vformat(fmt, std::make_format_args(args...));
            std::cout << prepend << message << "\n";
        }
        catch (const std::format_error& error)
        {
            std::cout << "log error: " << error.what() << ", call from " << loc.file_name() << " -> "
                      << loc.function_name() << "[" << loc.line() << ":" << loc.column() << "]\n";
        }
        catch (const std::exception& error)
        {
            std::cout << "log error: " << error.what() << ", call from " << loc.file_name() << " -> "
                      << loc.function_name() << "[" << loc.line() << ":" << loc.column() << "]\n";
        }

        if (level == Level::FATAL)
        {
            exit(1);
        }
    }
};

#define LOG_IMPL(name, levelName)                                                                            \
    template <typename... Args>                                                                              \
    struct name                                                                                              \
    {                                                                                                        \
        name(std::string_view fmt, Args... args, std::source_location loc = std::source_location::current()) \
        {                                                                                                    \
            internal(Level::levelName, Type::NORMAL, fmt, loc, std::forward<Args>(args)...);                 \
        }                                                                                                    \
    };                                                                                                       \
    template <typename... Args>                                                                              \
    name(std::string_view, Args...) -> name<Args...>;                                                        \
                                                                                                             \
    namespace loc {                                                                                          \
    template <typename... Args>                                                                              \
    struct name                                                                                              \
    {                                                                                                        \
        name(std::string_view fmt, Args... args, std::source_location loc = std::source_location::current()) \
        {                                                                                                    \
            internal(Level::levelName, Type::LOCATION, fmt, loc, std::forward<Args>(args)...);               \
        }                                                                                                    \
    };                                                                                                       \
    template <typename... Args>                                                                              \
    name(std::string_view, Args...) -> name<Args...>;                                                        \
    }                                                                                                        \
                                                                                                             \
    namespace func {                                                                                         \
    template <typename... Args>                                                                              \
    struct name                                                                                              \
    {                                                                                                        \
        name(std::string_view fmt, Args... args, std::source_location loc = std::source_location::current()) \
        {                                                                                                    \
            internal(Level::levelName, Type::FUNCTION, fmt, loc, std::forward<Args>(args)...);               \
        }                                                                                                    \
    };                                                                                                       \
    template <typename... Args>                                                                              \
    name(std::string_view, Args...) -> name<Args...>;                                                        \
    }

LOG_IMPL(debug, DEBUG_)
LOG_IMPL(info, INFO)
LOG_IMPL(warn, WARNING)
LOG_IMPL(error, ERROR)
LOG_IMPL(fatal, FATAL)

#undef LOG_IMPL

} // namespace huedra::log