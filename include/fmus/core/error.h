#pragma once

#include "../fmus_config.h"
#include <string>
#include <cstdint>

namespace fmus {
namespace core {

/**
 * @brief Error codes for the fmus-embed library
 */
enum class ErrorCode : uint32_t {
    // General errors
    Ok = 0,                    ///< No error
    Unknown = 1,               ///< Unknown error
    InvalidArgument = 2,       ///< Invalid argument provided
    NotImplemented = 3,        ///< Feature not implemented
    NotSupported = 4,          ///< Feature not supported
    Timeout = 5,               ///< Operation timed out
    ResourceUnavailable = 6,   ///< Resource unavailable
    InsufficientMemory = 7,    ///< Not enough memory
    NotInitialized = 8,        ///< Component not initialized
    DataError = 9,             ///< Data error

    // MCU errors
    McuInitFailed = 1000,      ///< MCU initialization failed
    PinConfigError = 1001,     ///< Error configuring a pin
    TimerError = 1002,         ///< Timer error
    AdcError = 1003,           ///< ADC error

    // Sensor errors
    SensorInitFailed = 2000,   ///< Sensor initialization failed
    SensorReadError = 2001,    ///< Error reading from sensor
    SensorCalibrationError = 2002, ///< Error calibrating sensor

    // Actuator errors
    ActuatorInitFailed = 3000, ///< Actuator initialization failed
    ActuatorSetValueError = 3001, ///< Error setting actuator value

    // Communication errors
    CommInitFailed = 4000,     ///< Communication initialization failed
    CommTransmitError = 4001,  ///< Error transmitting data
    CommReceiveError = 4002,   ///< Error receiving data
    CommConnectionError = 4003, ///< Connection error

    // DSP errors
    DspInitFailed = 5000,      ///< DSP initialization failed
    DspComputationError = 5001, ///< Error in DSP computation

    // AI errors
    AiInitFailed = 6000,       ///< AI initialization failed
    AiModelError = 6001,       ///< Error in AI model

    // Network errors
    NetworkInitFailed = 7000,  ///< Network initialization failed
    NetworkConnectionError = 7001, ///< Network connection error
    NetworkProtocolError = 7002, ///< Network protocol error

    // GPIO errors
    GPIOError = 8000,          ///< General GPIO error
    GPIOInitFailed = 8001,     ///< GPIO initialization failed
    GPIOWriteError = 8002,     ///< Error writing to GPIO
    GPIOReadError = 8003,      ///< Error reading from GPIO
    GPIOInterruptError = 8004  ///< Error with GPIO interrupt
};

/**
 * @brief Error class for handling and reporting errors
 */
class FMUS_EMBED_API Error {
public:
    /**
     * @brief Construct a new Error object
     *
     * @param code The error code
     * @param message The error message
     */
    Error(ErrorCode code, const std::string& message);

    /**
     * @brief Get the error code
     *
     * @return ErrorCode The error code
     */
    ErrorCode code() const;

    /**
     * @brief Get the error message
     *
     * @return const std::string& The error message
     */
    const std::string& message() const;

    /**
     * @brief Check if the error represents a successful operation
     *
     * @return true if the error code is Ok
     * @return false otherwise
     */
    bool isOk() const;

    /**
     * @brief Convert error to a human-readable string
     *
     * @return std::string Error as a string
     */
    std::string toString() const;

private:
    ErrorCode m_code;       ///< Error code
    std::string m_message;  ///< Error message
};

/**
 * @brief Get a string representation of an error code
 *
 * @param code The error code
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string errorCodeToString(ErrorCode code);

} // namespace core
} // namespace fmus
