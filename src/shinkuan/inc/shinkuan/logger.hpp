#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include <chrono>
#include <cstdarg>

// ANSI color codes
const std::string RED = "\033[1;31m";
const std::string YELLOW = "\033[1;33m";
const std::string GREEN = "\033[1;32m";
const std::string BLUE = "\033[1;34m";
const std::string RESET = "\033[0m";

enum LogLevel {
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
    DEBUG = 3
};

class Logger {
public:
    LogLevel logLevel_;
    static Logger& getInstance();

    void error(const char* format, ...);
    void warning(const char* format, ...);
    void info(const char* format, ...);
    void debug(const char* format, ...);

private:
    Logger();
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(const std::string& level, const std::string& color, const char* format, va_list args);

    std::chrono::steady_clock::time_point startTime_;
};

// Global logger instance
extern Logger& logger;

#endif // __LOGGER_H__
