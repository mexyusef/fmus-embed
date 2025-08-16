#include "fmus/actuators/servo.h"
#include "fmus/core/logging.h"
#include "fmus/gpio/gpio.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <map>
#include <memory>

namespace fmus {
namespace actuators {

// Implementation structure for servo
struct ServoImpl {
    std::thread movementThread;
    bool movementActive;
    std::function<void(float)> positionCallback;
    std::vector<ServoMovement> currentSequence;
    bool sequenceLoop;
    size_t sequenceIndex;
};

// Implementation structure for servo controller
struct ServoControllerImpl {
    std::map<std::string, std::shared_ptr<Servo>> servos;
    uint32_t nextId;
};

//=============================================================================
// Servo Implementation
//=============================================================================

Servo::Servo(uint8_t pwmPin, const ServoConfig& config)
    : m_pwmPin(pwmPin),
      m_config(config),
      m_initialized(false),
      m_currentAngle(90.0f),
      m_targetAngle(90.0f),
      m_currentPulseWidth(1500),
      m_enabled(true),
      m_moving(false),
      m_impl(nullptr) {
    
    m_impl = new ServoImpl();
    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->movementActive = false;
    impl->sequenceLoop = false;
    impl->sequenceIndex = 0;
}

Servo::~Servo() {
    if (m_impl) {
        ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
        impl->movementActive = false;
        if (impl->movementThread.joinable()) {
            impl->movementThread.join();
        }
        delete impl;
    }
}

core::Result<void> Servo::init() {
    FMUS_LOG_INFO("Initializing servo on PWM pin " + std::to_string(m_pwmPin));

    gpio::GPIO pwmGpio(m_pwmPin);
    auto result = pwmGpio.init(gpio::GPIODirection::Output);
    if (result.isError()) {
        return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                   "Failed to initialize PWM pin: " + result.error().message());
    }

    // Set initial position
    m_currentAngle = (m_config.minAngle + m_config.maxAngle) / 2.0f;
    m_targetAngle = m_currentAngle;
    m_currentPulseWidth = angleToPulseWidth(m_currentAngle);

    m_initialized = true;
    FMUS_LOG_INFO("Servo initialized successfully at " + std::to_string(m_currentAngle) + "°");
    return core::makeOk();
}

bool Servo::isInitialized() const {
    return m_initialized;
}

core::Result<void> Servo::setAngle(float angle) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    if (!m_enabled) {
        return core::makeError<void>(core::ErrorCode::ActuatorSetValueError,
                                   "Servo is disabled");
    }

    // Clamp angle to valid range
    angle = std::clamp(angle, m_config.minAngle, m_config.maxAngle);
    
    m_targetAngle = angle;
    m_currentAngle = angle;
    m_currentPulseWidth = angleToPulseWidth(angle);

    // Set PWM output (simplified implementation)
    gpio::GPIO pwmGpio(m_pwmPin);
    // In real implementation, this would generate proper PWM signal
    pwmGpio.write(true);
    std::this_thread::sleep_for(std::chrono::microseconds(m_currentPulseWidth));
    pwmGpio.write(false);

    // Call position callback if set
    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    if (impl->positionCallback) {
        impl->positionCallback(m_currentAngle);
    }

    FMUS_LOG_DEBUG("Servo angle set to " + std::to_string(angle) + "°");
    return core::makeOk();
}

core::Result<void> Servo::setAngle(float angle, uint32_t durationMs) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    if (!m_enabled) {
        return core::makeError<void>(core::ErrorCode::ActuatorSetValueError,
                                   "Servo is disabled");
    }

    // Clamp angle to valid range
    angle = std::clamp(angle, m_config.minAngle, m_config.maxAngle);
    m_targetAngle = angle;

    if (durationMs == 0 || !m_config.enableSmoothing) {
        return setAngle(angle);
    }

    // Start smooth movement
    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->movementActive = true;
    m_moving = true;

    if (impl->movementThread.joinable()) {
        impl->movementThread.join();
    }

    impl->movementThread = std::thread([this, angle, durationMs]() {
        float startAngle = m_currentAngle;
        float angleRange = angle - startAngle;
        uint32_t steps = m_config.smoothingSteps;
        uint32_t stepDelay = durationMs / steps;

        ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
        
        for (uint32_t i = 0; i <= steps && impl->movementActive; ++i) {
            float progress = static_cast<float>(i) / static_cast<float>(steps);
            
            // Apply easing function (ease-in-out)
            if (progress < 0.5f) {
                progress = 2.0f * progress * progress;
            } else {
                progress = 1.0f - 2.0f * (1.0f - progress) * (1.0f - progress);
            }
            
            float currentAngle = startAngle + angleRange * progress;
            setAngle(currentAngle);
            
            if (i < steps) {
                std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));
            }
        }
        
        m_moving = false;
        impl->movementActive = false;
    });

    FMUS_LOG_DEBUG("Servo smooth movement to " + std::to_string(angle) + "° over " + 
                   std::to_string(durationMs) + "ms");
    return core::makeOk();
}

float Servo::getAngle() const {
    return m_currentAngle;
}

float Servo::getTargetAngle() const {
    return m_targetAngle;
}

bool Servo::isMoving() const {
    return m_moving;
}

core::Result<void> Servo::stop() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->movementActive = false;
    m_moving = false;

    if (impl->movementThread.joinable()) {
        impl->movementThread.join();
    }

    FMUS_LOG_INFO("Servo stopped at " + std::to_string(m_currentAngle) + "°");
    return core::makeOk();
}

core::Result<void> Servo::sweep(float startAngle, float endAngle, uint32_t duration, uint32_t cycles) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    FMUS_LOG_INFO("Servo sweeping from " + std::to_string(startAngle) + "° to " + 
                  std::to_string(endAngle) + "° for " + std::to_string(cycles) + " cycles");

    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->movementActive = true;
    m_moving = true;

    if (impl->movementThread.joinable()) {
        impl->movementThread.join();
    }

    impl->movementThread = std::thread([this, startAngle, endAngle, duration, cycles]() {
        ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
        uint32_t cycleCount = 0;
        
        while ((cycles == 0 || cycleCount < cycles) && impl->movementActive) {
            // Move to start position
            setAngle(startAngle, duration / 2);
            std::this_thread::sleep_for(std::chrono::milliseconds(duration / 2));
            
            if (!impl->movementActive) break;
            
            // Move to end position
            setAngle(endAngle, duration / 2);
            std::this_thread::sleep_for(std::chrono::milliseconds(duration / 2));
            
            cycleCount++;
        }
        
        m_moving = false;
        impl->movementActive = false;
    });

    return core::makeOk();
}

core::Result<void> Servo::executeSequence(const std::vector<ServoMovement>& movements, bool loop) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    if (movements.empty()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Movement sequence is empty");
    }

    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->currentSequence = movements;
    impl->sequenceLoop = loop;
    impl->sequenceIndex = 0;
    impl->movementActive = true;
    m_moving = true;

    if (impl->movementThread.joinable()) {
        impl->movementThread.join();
    }

    impl->movementThread = std::thread([this]() {
        ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
        
        do {
            for (size_t i = 0; i < impl->currentSequence.size() && impl->movementActive; ++i) {
                const ServoMovement& movement = impl->currentSequence[i];
                setAngle(movement.targetAngle, movement.duration);
                std::this_thread::sleep_for(std::chrono::milliseconds(movement.duration));
            }
        } while (impl->sequenceLoop && impl->movementActive);
        
        m_moving = false;
        impl->movementActive = false;
    });

    FMUS_LOG_INFO("Servo sequence started with " + std::to_string(movements.size()) + " movements");
    return core::makeOk();
}

core::Result<void> Servo::setPulseWidth(uint16_t pulseWidth) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    // Clamp pulse width to valid range
    pulseWidth = std::clamp(pulseWidth, m_config.minPulseWidth, m_config.maxPulseWidth);
    m_currentPulseWidth = pulseWidth;
    m_currentAngle = pulseWidthToAngle(pulseWidth);
    m_targetAngle = m_currentAngle;

    FMUS_LOG_DEBUG("Servo pulse width set to " + std::to_string(pulseWidth) + " µs");
    return core::makeOk();
}

uint16_t Servo::getPulseWidth() const {
    return m_currentPulseWidth;
}

core::Result<void> Servo::calibrate() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Servo not initialized");
    }

    FMUS_LOG_INFO("Starting servo calibration");

    // Move to minimum position
    setAngle(m_config.minAngle);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Move to maximum position
    setAngle(m_config.maxAngle);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Return to center
    setAngle((m_config.minAngle + m_config.maxAngle) / 2.0f);

    FMUS_LOG_INFO("Servo calibration completed");
    return core::makeOk();
}

core::Result<void> Servo::setSpeed(uint32_t degreesPerSecond) {
    m_config.maxSpeed = degreesPerSecond;
    FMUS_LOG_DEBUG("Servo speed set to " + std::to_string(degreesPerSecond) + "°/s");
    return core::makeOk();
}

core::Result<void> Servo::setEnabled(bool enabled) {
    m_enabled = enabled;

    if (!enabled) {
        // Stop any current movement
        stop();
    }

    FMUS_LOG_DEBUG("Servo " + std::string(enabled ? "enabled" : "disabled"));
    return core::makeOk();
}

bool Servo::isEnabled() const {
    return m_enabled;
}

const ServoConfig& Servo::getConfig() const {
    return m_config;
}

uint8_t Servo::getPWMPin() const {
    return m_pwmPin;
}

std::string Servo::getStatus() const {
    std::ostringstream oss;
    oss << "Servo Status:\n";
    oss << "  PWM Pin: " << static_cast<int>(m_pwmPin) << "\n";
    oss << "  Type: " << servoTypeToString(m_config.type) << "\n";
    oss << "  Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    oss << "  Enabled: " << (m_enabled ? "Yes" : "No") << "\n";
    oss << "  Current Angle: " << m_currentAngle << "°\n";
    oss << "  Target Angle: " << m_targetAngle << "°\n";
    oss << "  Moving: " << (m_moving ? "Yes" : "No") << "\n";
    oss << "  Pulse Width: " << m_currentPulseWidth << " µs\n";
    oss << "  Range: " << m_config.minAngle << "° to " << m_config.maxAngle << "°\n";
    oss << "  Max Speed: " << m_config.maxSpeed << "°/s";
    return oss.str();
}

core::Result<void> Servo::setPositionCallback(std::function<void(float)> callback) {
    ServoImpl* impl = static_cast<ServoImpl*>(m_impl);
    impl->positionCallback = callback;
    return core::makeOk();
}

uint16_t Servo::angleToPulseWidth(float angle) const {
    float ratio = (angle - m_config.minAngle) / (m_config.maxAngle - m_config.minAngle);
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    return static_cast<uint16_t>(m_config.minPulseWidth +
                                ratio * (m_config.maxPulseWidth - m_config.minPulseWidth));
}

float Servo::pulseWidthToAngle(uint16_t pulseWidth) const {
    float ratio = static_cast<float>(pulseWidth - m_config.minPulseWidth) /
                  static_cast<float>(m_config.maxPulseWidth - m_config.minPulseWidth);
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    return m_config.minAngle + ratio * (m_config.maxAngle - m_config.minAngle);
}

void Servo::updatePosition() {
    // This would be called by a timer or interrupt in a real implementation
    // For now, it's a placeholder for position updates
}

//=============================================================================
// ServoController Implementation
//=============================================================================

ServoController::ServoController() : m_impl(nullptr) {
    m_impl = new ServoControllerImpl();
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    impl->nextId = 1;
}

ServoController::~ServoController() {
    if (m_impl) {
        delete static_cast<ServoControllerImpl*>(m_impl);
    }
}

core::Result<void> ServoController::addServo(std::shared_ptr<Servo> servo, const std::string& name) {
    if (!servo) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Servo pointer is null");
    }

    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    std::string servoName = name.empty() ? ("servo_" + std::to_string(impl->nextId++)) : name;

    if (impl->servos.find(servoName) != impl->servos.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Servo with name '" + servoName + "' already exists");
    }

    impl->servos[servoName] = servo;
    FMUS_LOG_INFO("Added servo '" + servoName + "' to controller");
    return core::makeOk();
}

core::Result<void> ServoController::removeServo(const std::string& name) {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    auto it = impl->servos.find(name);

    if (it == impl->servos.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Servo '" + name + "' not found");
    }

    // Stop servo before removing
    it->second->stop();
    impl->servos.erase(it);

    FMUS_LOG_INFO("Removed servo '" + name + "' from controller");
    return core::makeOk();
}

core::Result<void> ServoController::setServoAngle(const std::string& name, float angle, uint32_t duration) {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    auto it = impl->servos.find(name);

    if (it == impl->servos.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Servo '" + name + "' not found");
    }

    if (duration > 0) {
        return it->second->setAngle(angle, duration);
    } else {
        return it->second->setAngle(angle);
    }
}

core::Result<void> ServoController::executeCoordinatedMovement(const std::map<std::string, float>& movements, uint32_t duration) {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);

    // Start all movements simultaneously
    for (const auto& movement : movements) {
        const std::string& servoName = movement.first;
        float targetAngle = movement.second;

        auto it = impl->servos.find(servoName);
        if (it == impl->servos.end()) {
            return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                       "Servo '" + servoName + "' not found in coordinated movement");
        }

        auto result = it->second->setAngle(targetAngle, duration);
        if (result.isError()) {
            return result;
        }
    }

    FMUS_LOG_INFO("Coordinated movement started for " + std::to_string(movements.size()) + " servos");
    return core::makeOk();
}

core::Result<void> ServoController::stopAll() {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);

    for (auto& pair : impl->servos) {
        auto result = pair.second->stop();
        if (result.isError()) {
            FMUS_LOG_ERROR("Failed to stop servo '" + pair.first + "': " + result.error().message());
        }
    }

    FMUS_LOG_INFO("All servos stopped");
    return core::makeOk();
}

bool ServoController::isAnyMoving() const {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);

    for (const auto& pair : impl->servos) {
        if (pair.second->isMoving()) {
            return true;
        }
    }

    return false;
}

size_t ServoController::getServoCount() const {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    return impl->servos.size();
}

std::shared_ptr<Servo> ServoController::getServo(const std::string& name) const {
    ServoControllerImpl* impl = static_cast<ServoControllerImpl*>(m_impl);
    auto it = impl->servos.find(name);
    return (it != impl->servos.end()) ? it->second : nullptr;
}

//=============================================================================
// Helper Functions
//=============================================================================

std::string servoTypeToString(ServoType type) {
    switch (type) {
        case ServoType::Standard: return "Standard";
        case ServoType::Continuous: return "Continuous";
        case ServoType::Digital: return "Digital";
        case ServoType::Linear: return "Linear";
        default: return "Unknown";
    }
}

} // namespace actuators
} // namespace fmus
