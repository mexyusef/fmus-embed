#pragma once

#include "../fmus_config.h"
#include <string>
#include <functional>
#include <memory>
#include <cstdint>

namespace fmus {
namespace core {

/**
 * @brief Log levels for controlling logging verbosity
 */
enum class LogLevel : uint8_t {
    Trace = 0,   ///< Trace level (most verbose)
    Debug = 1,   ///< Debug level
    Info = 2,    ///< Information level
    Warning = 3, ///< Warning level
    Error = 4,   ///< Error level
    Fatal = 5,   ///< Fatal level (least verbose)
    None = 255   ///< No logging
};

/**
 * @brief A log message with metadata
 */
struct LogMessage {
    LogLevel level;        ///< The log level
    std::string message;   ///< The log message
    std::string file;      ///< Source file where the log was generated
    int line;              ///< Line number where the log was generated
    std::string function;  ///< Function where the log was generated
    uint64_t timestamp;    ///< Timestamp when the log was generated
};

/**
 * @brief Logger interface for handling log messages
 */
class FMUS_EMBED_API ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Log a message
     *
     * @param message The log message to handle
     */
    virtual void log(const LogMessage& message) = 0;

    /**
     * @brief Get the minimum log level that will be processed
     *
     * @return LogLevel The minimum log level
     */
    virtual LogLevel getLevel() const = 0;

    /**
     * @brief Set the minimum log level that will be processed
     *
     * @param level The minimum log level
     */
    virtual void setLevel(LogLevel level) = 0;
};

/**
 * @brief Default console logger implementation
 */
class FMUS_EMBED_API ConsoleLogger : public ILogger {
public:
    /**
     * @brief Construct a new Console Logger
     *
     * @param level The minimum log level to process
     */
    ConsoleLogger(LogLevel level = LogLevel::Info);

    /**
     * @brief Log a message to the console
     *
     * @param message The log message to handle
     */
    void log(const LogMessage& message) override;

    /**
     * @brief Get the minimum log level that will be processed
     *
     * @return LogLevel The minimum log level
     */
    LogLevel getLevel() const override;

    /**
     * @brief Set the minimum log level that will be processed
     *
     * @param level The minimum log level
     */
    void setLevel(LogLevel level) override;

private:
    LogLevel m_level;  ///< The minimum log level to process
};

/**
 * @brief Global logging system
 */
class FMUS_EMBED_API Logger {
public:
    /**
     * @brief Get the singleton instance of the Logger
     *
     * @return Logger& The logger instance
     */
    static Logger& instance();

    /**
     * @brief Set the current logger
     *
     * @param logger The logger to use
     */
    void setLogger(std::shared_ptr<ILogger> logger);

    /**
     * @brief Get the current logger
     *
     * @return std::shared_ptr<ILogger> The current logger
     */
    std::shared_ptr<ILogger> getLogger() const;

    /**
     * @brief Log a message
     *
     * @param level The log level
     * @param message The log message
     * @param file The source file
     * @param line The line number
     * @param function The function name
     */
    void log(LogLevel level, const std::string& message, const char* file, int line, const char* function);

private:
    Logger();  ///< Private constructor for singleton
    std::shared_ptr<ILogger> m_logger;  ///< The current logger
};

/**
 * @brief Convert a log level to a string
 *
 * @param level The log level
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string logLevelToString(LogLevel level);

/**
 * @brief Get the current timestamp in milliseconds
 *
 * @return uint64_t The timestamp
 */
FMUS_EMBED_API uint64_t getTimestamp();

} // namespace core
} // namespace fmus

// Convenience macros for logging
#define FMUS_LOG_TRACE(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Trace, message, __FILE__, __LINE__, __FUNCTION__)

#define FMUS_LOG_DEBUG(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Debug, message, __FILE__, __LINE__, __FUNCTION__)

#define FMUS_LOG_INFO(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Info, message, __FILE__, __LINE__, __FUNCTION__)

#define FMUS_LOG_WARNING(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Warning, message, __FILE__, __LINE__, __FUNCTION__)

#define FMUS_LOG_ERROR(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Error, message, __FILE__, __LINE__, __FUNCTION__)

#define FMUS_LOG_FATAL(message) \
    fmus::core::Logger::instance().log(fmus::core::LogLevel::Fatal, message, __FILE__, __LINE__, __FUNCTION__)
