#include <fmus/core/error.h>
#include <unordered_map>

namespace fmus {
namespace core {

// Mapping of error codes to string descriptions
static const std::unordered_map<ErrorCode, const char*> errorMessages = {
    // General errors
    { ErrorCode::Ok, "No error" },
    { ErrorCode::Unknown, "Unknown error" },
    { ErrorCode::InvalidArgument, "Invalid argument provided" },
    { ErrorCode::NotImplemented, "Feature not implemented" },
    { ErrorCode::NotSupported, "Feature not supported" },
    { ErrorCode::Timeout, "Operation timed out" },
    { ErrorCode::ResourceUnavailable, "Resource unavailable" },
    { ErrorCode::InsufficientMemory, "Not enough memory" },

    // MCU errors
    { ErrorCode::McuInitFailed, "MCU initialization failed" },
    { ErrorCode::PinConfigError, "Error configuring a pin" },
    { ErrorCode::TimerError, "Timer error" },
    { ErrorCode::AdcError, "ADC error" },

    // Sensor errors
    { ErrorCode::SensorInitFailed, "Sensor initialization failed" },
    { ErrorCode::SensorReadError, "Error reading from sensor" },
    { ErrorCode::SensorCalibrationError, "Error calibrating sensor" },

    // Actuator errors
    { ErrorCode::ActuatorInitFailed, "Actuator initialization failed" },
    { ErrorCode::ActuatorSetValueError, "Error setting actuator value" },

    // Communication errors
    { ErrorCode::CommInitFailed, "Communication initialization failed" },
    { ErrorCode::CommTransmitError, "Error transmitting data" },
    { ErrorCode::CommReceiveError, "Error receiving data" },
    { ErrorCode::CommConnectionError, "Connection error" },

    // DSP errors
    { ErrorCode::DspInitFailed, "DSP initialization failed" },
    { ErrorCode::DspComputationError, "Error in DSP computation" },

    // AI errors
    { ErrorCode::AiInitFailed, "AI initialization failed" },
    { ErrorCode::AiModelError, "Error in AI model" },

    // Network errors
    { ErrorCode::NetworkInitFailed, "Network initialization failed" },
    { ErrorCode::NetworkConnectionError, "Network connection error" },
    { ErrorCode::NetworkProtocolError, "Network protocol error" },

    // GPIO errors
    { ErrorCode::GPIOError, "General GPIO error" },
    { ErrorCode::GPIOInitFailed, "GPIO initialization failed" },
    { ErrorCode::GPIOWriteError, "Error writing to GPIO" },
    { ErrorCode::GPIOReadError, "Error reading from GPIO" },
    { ErrorCode::GPIOInterruptError, "Error with GPIO interrupt" }
};

Error::Error(ErrorCode code, const std::string& message)
    : m_code(code), m_message(message)
{
    // Jika pesan kosong, gunakan pesan default
    if (m_message.empty()) {
        auto it = errorMessages.find(code);
        if (it != errorMessages.end()) {
            m_message = it->second;
        } else {
            m_message = "Unknown error";
        }
    }
}

ErrorCode Error::code() const
{
    return m_code;
}

const std::string& Error::message() const
{
    return m_message;
}

bool Error::isOk() const
{
    return m_code == ErrorCode::Ok;
}

std::string Error::toString() const
{
    return errorCodeToString(m_code) + ": " + m_message;
}

std::string errorCodeToString(ErrorCode code)
{
    auto it = errorMessages.find(code);
    if (it != errorMessages.end()) {
        return it->second;
    }

    return "Unknown error code: " + std::to_string(static_cast<uint32_t>(code));
}

} // namespace core
} // namespace fmus
