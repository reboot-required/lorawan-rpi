/**
 * @file logger.h
 * @brief Lightweight console logger API.
 */

#ifndef LORAWAN_RPI_INCLUDE_LOGGER_H_
#define LORAWAN_RPI_INCLUDE_LOGGER_H_

#include <string>

namespace lorawan
{

/**
 * @brief Logging severity levels.
 */
enum class LogLevel
{
    kDebug   = 0,
    kInfo    = 1,
    kWarning = 2,
    kError   = 3,
};

/**
 * @brief Static console logger with timestamped output.
 */
class Logger
{
   public:
    /**
     * @brief Set minimum log level.
     * @param level Minimum level to print.
     */
    static void SetLevel(LogLevel level);

    /**
     * @brief Log a debug message.
     * @param message Message text.
     */
    static void Debug(const std::string& message);

    /**
     * @brief Log an informational message.
     * @param message Message text.
     */
    static void Info(const std::string& message);

    /**
     * @brief Log a warning message.
     * @param message Message text.
     */
    static void Warning(const std::string& message);

    /**
     * @brief Log an error message.
     * @param message Message text.
     */
    static void Error(const std::string& message);

   private:
    static LogLevel    current_level_;
    static void        Log(LogLevel level, const std::string& message);
    static const char* LevelToString(LogLevel level);
};

}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_LOGGER_H_