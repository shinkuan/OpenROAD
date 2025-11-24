#include "shinkuan/logger.hpp"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>

Logger::Logger() : startTime_(std::chrono::steady_clock::now()) {
    const char* logLevelStr = std::getenv("LOG_LEVEL");
    if (logLevelStr) {
        try {
            int level = std::stoi(logLevelStr);
            if (level == -1) {
                logLevel_ = static_cast<LogLevel>(level);
            } else if (level >= static_cast<int>(LogLevel::ERROR) && level <= static_cast<int>(LogLevel::DEBUG)) {
                logLevel_ = static_cast<LogLevel>(level);
            } else {
                logLevel_ = LogLevel::INFO;
            }
        } catch (...) {
            logLevel_ = LogLevel::INFO;
        }
    } else {
        logLevel_ = LogLevel::INFO;
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& level, const std::string& color, const char* format, va_list args) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);

    long long total_ms = elapsed.count();
    int ms = total_ms % 1000;
    int total_seconds = total_ms / 1000;
    int seconds = total_seconds % 60;
    int total_minutes = total_seconds / 60;
    int minutes = total_minutes % 60;
    int hours = total_minutes / 60;

    // [HH:MM:SS.XX]
    char time_buf[32];
    snprintf(time_buf, sizeof(time_buf), "[%02d:%02d:%02d.%02d]", hours, minutes, seconds, ms / 10);

    // [LEVEL]
    std::cout << color << level << RESET << " " << time_buf << " ";
    
    vprintf(format, args);
    printf("\n");
}

void Logger::error(const char* format, ...) {
    if (logLevel_ < LogLevel::ERROR) return;
    va_list args;
    va_start(args, format);
    log("[ERROR]", RED, format, args);
    va_end(args);
}

void Logger::warning(const char* format, ...) {
    if (logLevel_ < LogLevel::WARNING) return;
    va_list args;
    va_start(args, format);
    log("[WARNING]", YELLOW, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    if (logLevel_ < LogLevel::INFO) return;
    va_list args;
    va_start(args, format);
    log("[INFO]", GREEN, format, args);
    va_end(args);
}

void Logger::debug(const char* format, ...) {
    if (logLevel_ < LogLevel::DEBUG) return;
    va_list args;
    va_start(args, format);
    log("[DEBUG]", BLUE, format, args);
    va_end(args);
}

// Define the global logger instance
Logger& logger = Logger::getInstance();
