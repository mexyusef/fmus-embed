#pragma once

/**
 * @file servo.h
 * @brief Servo control interface for the fmus-embed library
 *
 * This header provides specialized servo control functionality with
 * advanced features like multi-servo coordination, smooth movements,
 * and position feedback.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace fmus {
namespace actuators {

/**
 * @brief Servo types enumeration
 */
enum class ServoType : uint8_t {
    Standard = 0,    ///< Standard 180-degree servo
    Continuous = 1,  ///< Continuous rotation servo
    Digital = 2,     ///< Digital servo with feedback
    Linear = 3       ///< Linear servo actuator
};

/**
 * @brief Servo configuration structure
 */
struct ServoConfig {
    ServoType type;                 ///< Servo type
    uint16_t minPulseWidth;         ///< Minimum pulse width (µs)
    uint16_t maxPulseWidth;         ///< Maximum pulse width (µs)
    uint16_t centerPulseWidth;      ///< Center pulse width (µs)
    float minAngle;                 ///< Minimum angle (degrees)
    float maxAngle;                 ///< Maximum angle (degrees)
    uint32_t pwmFrequency;          ///< PWM frequency (Hz)
    bool enableSmoothing;           ///< Enable smooth movement
    uint32_t smoothingSteps;        ///< Number of smoothing steps
    uint32_t maxSpeed;              ///< Maximum speed (degrees/second)

    /**
     * @brief Constructor with default values
     */
    ServoConfig(ServoType servoType = ServoType::Standard,
                uint16_t minPulse = 1000,
                uint16_t maxPulse = 2000,
                uint16_t centerPulse = 1500,
                float minAng = 0.0f,
                float maxAng = 180.0f,
                uint32_t frequency = 50,
                bool smoothing = true,
                uint32_t steps = 20,
                uint32_t speed = 180)
        : type(servoType), minPulseWidth(minPulse), maxPulseWidth(maxPulse),
          centerPulseWidth(centerPulse), minAngle(minAng), maxAngle(maxAng),
          pwmFrequency(frequency), enableSmoothing(smoothing),
          smoothingSteps(steps), maxSpeed(speed) {}
};

/**
 * @brief Servo movement profile
 */
struct ServoMovement {
    float targetAngle;      ///< Target angle
    uint32_t duration;      ///< Movement duration (ms)
    bool useEasing;         ///< Use easing function
    std::function<float(float)> easingFunction; ///< Custom easing function

    ServoMovement(float angle, uint32_t dur, bool easing = true)
        : targetAngle(angle), duration(dur), useEasing(easing) {}
};

/**
 * @brief Servo control class
 */
class FMUS_EMBED_API Servo {
public:
    /**
     * @brief Construct a new Servo
     *
     * @param pwmPin PWM control pin
     * @param config Servo configuration
     */
    explicit Servo(uint8_t pwmPin, const ServoConfig& config = ServoConfig());

    /**
     * @brief Destructor
     */
    ~Servo();

    /**
     * @brief Initialize the servo
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init();

    /**
     * @brief Check if the servo is initialized
     *
     * @return bool True if initialized
     */
    bool isInitialized() const;

    /**
     * @brief Set servo angle
     *
     * @param angle Angle in degrees
     * @return core::Result<void> Success or error
     */
    core::Result<void> setAngle(float angle);

    /**
     * @brief Set servo angle with duration
     *
     * @param angle Target angle in degrees
     * @param durationMs Movement duration in milliseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> setAngle(float angle, uint32_t durationMs);

    /**
     * @brief Get current angle
     *
     * @return float Current angle in degrees
     */
    float getAngle() const;

    /**
     * @brief Get target angle (during movement)
     *
     * @return float Target angle in degrees
     */
    float getTargetAngle() const;

    /**
     * @brief Check if servo is moving
     *
     * @return bool True if currently moving
     */
    bool isMoving() const;

    /**
     * @brief Stop current movement
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> stop();

    /**
     * @brief Sweep between two angles
     *
     * @param startAngle Start angle in degrees
     * @param endAngle End angle in degrees
     * @param duration Total duration in milliseconds
     * @param cycles Number of sweep cycles (0 for infinite)
     * @return core::Result<void> Success or error
     */
    core::Result<void> sweep(float startAngle, float endAngle, uint32_t duration, uint32_t cycles = 1);

    /**
     * @brief Execute a sequence of movements
     *
     * @param movements Vector of servo movements
     * @param loop True to loop the sequence
     * @return core::Result<void> Success or error
     */
    core::Result<void> executeSequence(const std::vector<ServoMovement>& movements, bool loop = false);

    /**
     * @brief Set pulse width directly
     *
     * @param pulseWidth Pulse width in microseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> setPulseWidth(uint16_t pulseWidth);

    /**
     * @brief Get current pulse width
     *
     * @return uint16_t Current pulse width in microseconds
     */
    uint16_t getPulseWidth() const;

    /**
     * @brief Calibrate servo (find actual min/max positions)
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate();

    /**
     * @brief Set movement speed
     *
     * @param degreesPerSecond Speed in degrees per second
     * @return core::Result<void> Success or error
     */
    core::Result<void> setSpeed(uint32_t degreesPerSecond);

    /**
     * @brief Enable/disable servo (power saving)
     *
     * @param enabled True to enable, false to disable
     * @return core::Result<void> Success or error
     */
    core::Result<void> setEnabled(bool enabled);

    /**
     * @brief Check if servo is enabled
     *
     * @return bool True if enabled
     */
    bool isEnabled() const;

    /**
     * @brief Get servo configuration
     *
     * @return const ServoConfig& Current configuration
     */
    const ServoConfig& getConfig() const;

    /**
     * @brief Get PWM pin number
     *
     * @return uint8_t PWM pin number
     */
    uint8_t getPWMPin() const;

    /**
     * @brief Get servo status
     *
     * @return std::string Status information
     */
    std::string getStatus() const;

    /**
     * @brief Set position change callback
     *
     * @param callback Callback function called when position changes
     * @return core::Result<void> Success or error
     */
    core::Result<void> setPositionCallback(std::function<void(float)> callback);

private:
    uint8_t m_pwmPin;               ///< PWM control pin
    ServoConfig m_config;           ///< Servo configuration
    bool m_initialized;             ///< Initialization state
    float m_currentAngle;           ///< Current angle
    float m_targetAngle;            ///< Target angle
    uint16_t m_currentPulseWidth;   ///< Current pulse width
    bool m_enabled;                 ///< Enable state
    bool m_moving;                  ///< Movement state
    void* m_impl;                   ///< Platform-specific implementation

    /**
     * @brief Convert angle to pulse width
     *
     * @param angle Angle in degrees
     * @return uint16_t Pulse width in microseconds
     */
    uint16_t angleToPulseWidth(float angle) const;

    /**
     * @brief Convert pulse width to angle
     *
     * @param pulseWidth Pulse width in microseconds
     * @return float Angle in degrees
     */
    float pulseWidthToAngle(uint16_t pulseWidth) const;

    /**
     * @brief Update servo position (called by movement thread)
     */
    void updatePosition();
};

/**
 * @brief Multi-servo controller for coordinated servo operations
 */
class FMUS_EMBED_API ServoController {
public:
    /**
     * @brief Constructor
     */
    ServoController();

    /**
     * @brief Destructor
     */
    ~ServoController();

    /**
     * @brief Add servo to controller
     *
     * @param servo Servo to add
     * @param name Optional name for the servo
     * @return core::Result<void> Success or error
     */
    core::Result<void> addServo(std::shared_ptr<Servo> servo, const std::string& name = "");

    /**
     * @brief Remove servo from controller
     *
     * @param name Servo name
     * @return core::Result<void> Success or error
     */
    core::Result<void> removeServo(const std::string& name);

    /**
     * @brief Set angle of named servo
     *
     * @param name Servo name
     * @param angle Target angle
     * @param duration Movement duration (optional)
     * @return core::Result<void> Success or error
     */
    core::Result<void> setServoAngle(const std::string& name, float angle, uint32_t duration = 0);

    /**
     * @brief Execute coordinated movement
     *
     * @param movements Map of servo names to target angles
     * @param duration Movement duration for all servos
     * @return core::Result<void> Success or error
     */
    core::Result<void> executeCoordinatedMovement(const std::map<std::string, float>& movements, uint32_t duration);

    /**
     * @brief Stop all servos
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> stopAll();

    /**
     * @brief Check if any servo is moving
     *
     * @return bool True if any servo is moving
     */
    bool isAnyMoving() const;

    /**
     * @brief Get number of servos
     *
     * @return size_t Number of servos
     */
    size_t getServoCount() const;

    /**
     * @brief Get servo by name
     *
     * @param name Servo name
     * @return std::shared_ptr<Servo> Servo pointer or nullptr
     */
    std::shared_ptr<Servo> getServo(const std::string& name) const;

private:
    void* m_impl; ///< Implementation details
};

/**
 * @brief Get string representation of servo type
 *
 * @param type Servo type
 * @return std::string String representation
 */
FMUS_EMBED_API std::string servoTypeToString(ServoType type);

} // namespace actuators
} // namespace fmus
