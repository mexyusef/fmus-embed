#include <fmus/core/logging.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <ctime>

namespace fmus {
namespace core {

// Convert log level to string
std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        case LogLevel::None:    return "NONE";
        default:                return "UNKNOWN";
    }
}

// Get current timestamp in milliseconds
uint64_t getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Format timestamp as a human-readable string
std::string formatTimestamp(uint64_t timestamp) {
    auto timePoint = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(timestamp));
    auto timeT = std::chrono::system_clock::to_time_t(timePoint);
    auto ms = timestamp % 1000;

    std::stringstream ss;
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &timeT);
#else
    localtime_r(&timeT, &tm);
#endif

    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "."
       << std::setfill('0') << std::setw(3) << ms;

    return ss.str();
}

// Extract filename from path
std::string extractFilename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

// ConsoleLogger implementation
ConsoleLogger::ConsoleLogger(LogLevel level)
    : m_level(level) {
}

void ConsoleLogger::log(const LogMessage& message) {
    if (message.level < m_level) {
        return; // Skip messages below the minimum level
    }

    // Format: [TIMESTAMP] [LEVEL] [FILE:LINE] MESSAGE
    std::string formattedMessage =
        "[" + formatTimestamp(message.timestamp) + "] " +
        "[" + logLevelToString(message.level) + "] " +
        "[" + extractFilename(message.file) + ":" + std::to_string(message.line) + "] " +
        message.message;

    // Output to the appropriate stream based on level
    if (message.level >= LogLevel::Error) {
        std::cerr << formattedMessage << std::endl;
    } else {
        std::cout << formattedMessage << std::endl;
    }
}

LogLevel ConsoleLogger::getLevel() const {
    return m_level;
}

void ConsoleLogger::setLevel(LogLevel level) {
    m_level = level;
}

// Logger singleton implementation
static std::mutex g_loggerMutex;

Logger::Logger()
    : m_logger(std::make_shared<ConsoleLogger>()) {
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::setLogger(std::shared_ptr<ILogger> logger) {
    if (!logger) {
        return; // Don't allow null loggers
    }

    std::lock_guard<std::mutex> lock(g_loggerMutex);
    m_logger = logger;
}

std::shared_ptr<ILogger> Logger::getLogger() const {
    std::lock_guard<std::mutex> lock(g_loggerMutex);
    return m_logger;
}

void Logger::log(LogLevel level, const std::string& message, const char* file, int line, const char* function) {
    std::shared_ptr<ILogger> logger;

    {
        std::lock_guard<std::mutex> lock(g_loggerMutex);
        logger = m_logger;
    }

    if (!logger || level < logger->getLevel()) {
        return; // Skip if no logger or below minimum level
    }

    LogMessage logMessage;
    logMessage.level = level;
    logMessage.message = message;
    logMessage.file = file ? file : "";
    logMessage.line = line;
    logMessage.function = function ? function : "";
    logMessage.timestamp = getTimestamp();

    logger->log(logMessage);
}

} // namespace core
} // namespace fmus
