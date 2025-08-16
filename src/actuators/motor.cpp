#include "fmus/actuators/motor.h"
#include "fmus/core/logging.h"
#include "fmus/gpio/gpio.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <thread>
#include <chrono>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

namespace fmus {
namespace actuators {

// Step sequences for stepper motor
static const uint8_t STEPPER_SEQUENCE_FULL[4][4] = {
    {1, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 0, 1}
};

static const uint8_t STEPPER_SEQUENCE_HALF[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

//=============================================================================
// DCMotor Implementation
//=============================================================================

DCMotor::DCMotor(uint8_t pwmPin, uint8_t directionPin, uint8_t enablePin)
    : m_pwmPin(pwmPin),
      m_directionPin(directionPin),
      m_enablePin(enablePin),
      m_initialized(false),
      m_speed(0.0f),
      m_direction(MotorDirection::Forward),
      m_enabled(true),
      m_pwmFrequency(1000) {
}

DCMotor::~DCMotor() {
    if (m_initialized) {
        stop();
    }
}

core::Result<void> DCMotor::init() {
    FMUS_LOG_INFO("Initializing DC motor on PWM pin " + std::to_string(m_pwmPin));

    // Initialize PWM pin
    gpio::GPIO pwmGpio(m_pwmPin);
    auto pwmResult = pwmGpio.init(gpio::GPIODirection::Output);
    if (pwmResult.isError()) {
        return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                   "Failed to initialize PWM pin: " + pwmResult.error().message());
    }

    // Initialize direction pin if specified
    if (m_directionPin != 255) {
        gpio::GPIO dirGpio(m_directionPin);
        auto dirResult = dirGpio.init(gpio::GPIODirection::Output);
        if (dirResult.isError()) {
            return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                       "Failed to initialize direction pin: " + dirResult.error().message());
        }
    }

    // Initialize enable pin if specified
    if (m_enablePin != 255) {
        gpio::GPIO enableGpio(m_enablePin);
        auto enableResult = enableGpio.init(gpio::GPIODirection::Output);
        if (enableResult.isError()) {
            return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                       "Failed to initialize enable pin: " + enableResult.error().message());
        }

        // Set enable pin high by default
        enableGpio.write(true);
    }

    m_initialized = true;
    FMUS_LOG_INFO("DC motor initialized successfully");
    return core::makeOk();
}

bool DCMotor::isInitialized() const {
    return m_initialized;
}

core::Result<void> DCMotor::stop() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Motor not initialized");
    }

    auto result = setSpeed(0.0f);
    if (result.isOk()) {
        FMUS_LOG_INFO("DC motor stopped");
    }
    return result;
}

MotorType DCMotor::getType() const {
    return MotorType::DC;
}

std::string DCMotor::getStatus() const {
    std::ostringstream oss;
    oss << "DC Motor Status:\n";
    oss << "  PWM Pin: " << static_cast<int>(m_pwmPin) << "\n";
    oss << "  Direction Pin: " << (m_directionPin == 255 ? "None" : std::to_string(m_directionPin)) << "\n";
    oss << "  Enable Pin: " << (m_enablePin == 255 ? "None" : std::to_string(m_enablePin)) << "\n";
    oss << "  Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    oss << "  Speed: " << (m_speed * 100.0f) << "%\n";
    oss << "  Direction: " << motorDirectionToString(m_direction) << "\n";
    oss << "  Enabled: " << (m_enabled ? "Yes" : "No") << "\n";
    oss << "  PWM Frequency: " << m_pwmFrequency << " Hz";
    return oss.str();
}

core::Result<void> DCMotor::setSpeed(float speed) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Motor not initialized");
    }

    // Clamp speed to valid range
    speed = std::clamp(speed, 0.0f, 1.0f);
    m_speed = speed;

    // Convert speed to PWM duty cycle (0-255 for 8-bit PWM)
    uint8_t pwmValue = static_cast<uint8_t>(speed * 255.0f);

    // Set PWM output (simplified - in real implementation would use hardware PWM)
    gpio::GPIO pwmGpio(m_pwmPin);
    if (pwmValue == 0) {
        pwmGpio.write(false);
    } else if (pwmValue == 255) {
        pwmGpio.write(true);
    } else {
        // For demonstration, use simple on/off based on duty cycle
        // Real implementation would use hardware PWM
        pwmGpio.write(speed > 0.5f);
    }

    FMUS_LOG_DEBUG("DC motor speed set to " + std::to_string(speed * 100.0f) + "%");
    return core::makeOk();
}

core::Result<void> DCMotor::setDirection(MotorDirection direction) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Motor not initialized");
    }

    if (m_directionPin == 255) {
        return core::makeError<void>(core::ErrorCode::NotSupported,
                                   "Direction control not available (no direction pin)");
    }

    m_direction = direction;

    gpio::GPIO dirGpio(m_directionPin);
    switch (direction) {
        case MotorDirection::Forward:
            dirGpio.write(false);
            break;
        case MotorDirection::Reverse:
            dirGpio.write(true);
            break;
        case MotorDirection::Brake:
            // For brake, stop the motor
            setSpeed(0.0f);
            break;
        case MotorDirection::Coast:
            // For coast, disable the motor
            setEnabled(false);
            break;
    }

    FMUS_LOG_DEBUG("DC motor direction set to " + motorDirectionToString(direction));
    return core::makeOk();
}

core::Result<void> DCMotor::setSpeedAndDirection(float speed) {
    if (speed >= 0.0f) {
        auto dirResult = setDirection(MotorDirection::Forward);
        if (dirResult.isError()) return dirResult;
        return setSpeed(speed);
    } else {
        auto dirResult = setDirection(MotorDirection::Reverse);
        if (dirResult.isError()) return dirResult;
        return setSpeed(-speed);
    }
}

core::Result<void> DCMotor::setEnabled(bool enabled) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Motor not initialized");
    }

    m_enabled = enabled;

    if (m_enablePin != 255) {
        gpio::GPIO enableGpio(m_enablePin);
        enableGpio.write(enabled);
    }

    FMUS_LOG_DEBUG("DC motor " + std::string(enabled ? "enabled" : "disabled"));
    return core::makeOk();
}

float DCMotor::getSpeed() const {
    return m_speed;
}

MotorDirection DCMotor::getDirection() const {
    return m_direction;
}

bool DCMotor::isEnabled() const {
    return m_enabled;
}

core::Result<void> DCMotor::setPWMFrequency(uint32_t frequency) {
    m_pwmFrequency = frequency;
    // In a real implementation, this would configure the hardware PWM frequency
    FMUS_LOG_DEBUG("DC motor PWM frequency set to " + std::to_string(frequency) + " Hz");
    return core::makeOk();
}

//=============================================================================
// ServoMotor Implementation
//=============================================================================

ServoMotor::ServoMotor(uint8_t pwmPin, uint16_t minPulseWidth, uint16_t maxPulseWidth)
    : m_pwmPin(pwmPin),
      m_minPulseWidth(minPulseWidth),
      m_maxPulseWidth(maxPulseWidth),
      m_initialized(false),
      m_currentAngle(90.0f),
      m_currentPulseWidth(1500) {
}

ServoMotor::~ServoMotor() {
    if (m_initialized) {
        stop();
    }
}

core::Result<void> ServoMotor::init() {
    FMUS_LOG_INFO("Initializing servo motor on PWM pin " + std::to_string(m_pwmPin));

    gpio::GPIO pwmGpio(m_pwmPin);
    auto result = pwmGpio.init(gpio::GPIODirection::Output);
    if (result.isError()) {
        return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                   "Failed to initialize PWM pin: " + result.error().message());
    }

    // Set initial position to center
    setAngle(90.0f);

    m_initialized = true;
    FMUS_LOG_INFO("Servo motor initialized successfully");
    return core::makeOk();
}

bool ServoMotor::isInitialized() const {
    return m_initialized;
}

core::Result<void> ServoMotor::stop() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    // For servo, "stop" means hold current position
    FMUS_LOG_INFO("Servo motor stopped at angle " + std::to_string(m_currentAngle));
    return core::makeOk();
}

MotorType ServoMotor::getType() const {
    return MotorType::Servo;
}

std::string ServoMotor::getStatus() const {
    std::ostringstream oss;
    oss << "Servo Motor Status:\n";
    oss << "  PWM Pin: " << static_cast<int>(m_pwmPin) << "\n";
    oss << "  Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    oss << "  Current Angle: " << m_currentAngle << "°\n";
    oss << "  Current Pulse Width: " << m_currentPulseWidth << " µs\n";
    oss << "  Min Pulse Width: " << m_minPulseWidth << " µs\n";
    oss << "  Max Pulse Width: " << m_maxPulseWidth << " µs";
    return oss.str();
}

core::Result<void> ServoMotor::setAngle(float angle) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    // Clamp angle to valid range
    angle = std::clamp(angle, 0.0f, 180.0f);
    m_currentAngle = angle;

    // Convert angle to pulse width
    float ratio = angle / 180.0f;
    m_currentPulseWidth = static_cast<uint16_t>(
        m_minPulseWidth + ratio * (m_maxPulseWidth - m_minPulseWidth));

    // Set PWM output (simplified implementation)
    gpio::GPIO pwmGpio(m_pwmPin);
    // In real implementation, this would generate proper PWM signal
    // For now, just indicate the servo is active
    pwmGpio.write(true);
    std::this_thread::sleep_for(std::chrono::microseconds(m_currentPulseWidth));
    pwmGpio.write(false);

    FMUS_LOG_DEBUG("Servo angle set to " + std::to_string(angle) + "° (pulse: " + 
                   std::to_string(m_currentPulseWidth) + " µs)");
    return core::makeOk();
}

float ServoMotor::getAngle() const {
    return m_currentAngle;
}

core::Result<void> ServoMotor::sweep(float startAngle, float endAngle, uint32_t duration) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    FMUS_LOG_INFO("Servo sweeping from " + std::to_string(startAngle) + "° to " + 
                  std::to_string(endAngle) + "° over " + std::to_string(duration) + "ms");

    const uint32_t steps = 50;
    const uint32_t stepDelay = duration / steps;
    const float angleStep = (endAngle - startAngle) / static_cast<float>(steps);

    for (uint32_t i = 0; i <= steps; ++i) {
        float currentAngle = startAngle + angleStep * static_cast<float>(i);
        auto result = setAngle(currentAngle);
        if (result.isError()) {
            return result;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
    }

    return core::makeOk();
}

core::Result<void> ServoMotor::setPulseWidth(uint16_t pulseWidth) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    // Clamp pulse width to valid range
    pulseWidth = std::clamp(pulseWidth, m_minPulseWidth, m_maxPulseWidth);
    m_currentPulseWidth = pulseWidth;

    // Convert pulse width to angle
    float ratio = static_cast<float>(pulseWidth - m_minPulseWidth) / 
                  static_cast<float>(m_maxPulseWidth - m_minPulseWidth);
    m_currentAngle = ratio * 180.0f;

    FMUS_LOG_DEBUG("Servo pulse width set to " + std::to_string(pulseWidth) + " µs (angle: " +
                   std::to_string(m_currentAngle) + "°)");
    return core::makeOk();
}

//=============================================================================
// StepperMotor Implementation
//=============================================================================

StepperMotor::StepperMotor(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4,
                           uint16_t stepsPerRevolution)
    : m_stepsPerRevolution(stepsPerRevolution),
      m_initialized(false),
      m_currentPosition(0),
      m_stepMode(StepMode::Full),
      m_stepDelay(1000),
      m_currentStep(0) {
    m_pins[0] = pin1;
    m_pins[1] = pin2;
    m_pins[2] = pin3;
    m_pins[3] = pin4;
}

StepperMotor::~StepperMotor() {
    if (m_initialized) {
        stop();
    }
}

core::Result<void> StepperMotor::init() {
    FMUS_LOG_INFO("Initializing stepper motor with pins " +
                  std::to_string(m_pins[0]) + ", " + std::to_string(m_pins[1]) + ", " +
                  std::to_string(m_pins[2]) + ", " + std::to_string(m_pins[3]));

    // Initialize all control pins
    for (int i = 0; i < 4; ++i) {
        gpio::GPIO pinGpio(m_pins[i]);
        auto result = pinGpio.init(gpio::GPIODirection::Output);
        if (result.isError()) {
            return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                       "Failed to initialize pin " + std::to_string(m_pins[i]) +
                                       ": " + result.error().message());
        }
        // Set all pins low initially
        pinGpio.write(false);
    }

    m_initialized = true;
    FMUS_LOG_INFO("Stepper motor initialized successfully");
    return core::makeOk();
}

bool StepperMotor::isInitialized() const {
    return m_initialized;
}

core::Result<void> StepperMotor::stop() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Stepper not initialized");
    }

    // Turn off all coils
    for (int i = 0; i < 4; ++i) {
        gpio::GPIO pinGpio(m_pins[i]);
        pinGpio.write(false);
    }

    FMUS_LOG_INFO("Stepper motor stopped");
    return core::makeOk();
}

MotorType StepperMotor::getType() const {
    return MotorType::Stepper;
}

std::string StepperMotor::getStatus() const {
    std::ostringstream oss;
    oss << "Stepper Motor Status:\n";
    oss << "  Control Pins: " << static_cast<int>(m_pins[0]) << ", "
        << static_cast<int>(m_pins[1]) << ", " << static_cast<int>(m_pins[2])
        << ", " << static_cast<int>(m_pins[3]) << "\n";
    oss << "  Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    oss << "  Steps per Revolution: " << m_stepsPerRevolution << "\n";
    oss << "  Current Position: " << m_currentPosition << " steps\n";
    oss << "  Step Mode: " << stepModeToString(m_stepMode) << "\n";
    oss << "  Step Delay: " << m_stepDelay << " µs";
    return oss.str();
}

core::Result<void> StepperMotor::step(int32_t steps) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Stepper not initialized");
    }

    FMUS_LOG_DEBUG("Stepper motor stepping " + std::to_string(steps) + " steps");

    int8_t direction = (steps > 0) ? 1 : -1;
    int32_t absSteps = std::abs(steps);

    for (int32_t i = 0; i < absSteps; ++i) {
        executeStep(direction);
        m_currentPosition += direction;
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepDelay));
    }

    return core::makeOk();
}

core::Result<void> StepperMotor::setStepMode(StepMode mode) {
    m_stepMode = mode;
    FMUS_LOG_DEBUG("Stepper step mode set to " + stepModeToString(mode));
    return core::makeOk();
}

core::Result<void> StepperMotor::setStepDelay(uint32_t delayMicroseconds) {
    m_stepDelay = delayMicroseconds;
    FMUS_LOG_DEBUG("Stepper step delay set to " + std::to_string(delayMicroseconds) + " µs");
    return core::makeOk();
}

core::Result<void> StepperMotor::rotate(float degrees) {
    int32_t steps = static_cast<int32_t>((degrees / 360.0f) * m_stepsPerRevolution);
    return step(steps);
}

int32_t StepperMotor::getPosition() const {
    return m_currentPosition;
}

core::Result<void> StepperMotor::resetPosition() {
    m_currentPosition = 0;
    FMUS_LOG_DEBUG("Stepper position reset to 0");
    return core::makeOk();
}

void StepperMotor::executeStep(int8_t direction) {
    const uint8_t (*sequence)[4];
    uint8_t sequenceLength;

    if (m_stepMode == StepMode::Half) {
        sequence = STEPPER_SEQUENCE_HALF;
        sequenceLength = 8;
    } else {
        sequence = STEPPER_SEQUENCE_FULL;
        sequenceLength = 4;
    }

    // Update step index
    if (direction > 0) {
        m_currentStep = (m_currentStep + 1) % sequenceLength;
    } else {
        m_currentStep = (m_currentStep + sequenceLength - 1) % sequenceLength;
    }

    // Set pin states according to sequence
    for (int i = 0; i < 4; ++i) {
        gpio::GPIO pinGpio(m_pins[i]);
        bool state = sequence[m_currentStep][i] != 0;
        pinGpio.write(state);
    }
}

//=============================================================================
// Helper Functions
//=============================================================================

std::string motorTypeToString(MotorType type) {
    switch (type) {
        case MotorType::DC: return "DC Motor";
        case MotorType::Servo: return "Servo Motor";
        case MotorType::Stepper: return "Stepper Motor";
        default: return "Unknown";
    }
}

std::string motorDirectionToString(MotorDirection direction) {
    switch (direction) {
        case MotorDirection::Forward: return "Forward";
        case MotorDirection::Reverse: return "Reverse";
        case MotorDirection::Brake: return "Brake";
        case MotorDirection::Coast: return "Coast";
        default: return "Unknown";
    }
}

std::string stepModeToString(StepMode mode) {
    switch (mode) {
        case StepMode::Full: return "Full Step";
        case StepMode::Half: return "Half Step";
        case StepMode::Quarter: return "Quarter Step";
        case StepMode::Eighth: return "Eighth Step";
        case StepMode::Sixteenth: return "Sixteenth Step";
        default: return "Unknown";
    }
}

} // namespace actuators
} // namespace fmus
