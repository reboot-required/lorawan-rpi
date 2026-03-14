// logger.cpp: timestamped console logger implementation.

#include "logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace lorawan
{

LogLevel Logger::current_level_ = LogLevel::kInfo;

void Logger::SetLevel(LogLevel level) { current_level_ = level; }

void Logger::Debug(const std::string& message) { Log(LogLevel::kDebug, message); }

void Logger::Info(const std::string& message) { Log(LogLevel::kInfo, message); }

void Logger::Warning(const std::string& message) { Log(LogLevel::kWarning, message); }

void Logger::Error(const std::string& message) { Log(LogLevel::kError, message); }

void Logger::Log(LogLevel level, const std::string& message)
{
    if (level < current_level_)
    {
        return;
    }

    auto now    = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_c), "%H:%M:%S") << "." << std::setfill('0')
        << std::setw(3) << ms.count() << " [" << LevelToString(level) << "] " << message;

    if (level >= LogLevel::kError)
    {
        std::cerr << oss.str() << std::endl;
    }
    else
    {
        std::cout << oss.str() << std::endl;
    }
}

const char* Logger::LevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO ";
        case LogLevel::kWarning:
            return "WARN ";
        case LogLevel::kError:
            return "ERROR";
    }
    return "?????";
}

}  // namespace lorawan