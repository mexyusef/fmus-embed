#pragma once

/**
 * @file motor.h
 * @brief Motor control interfaces for the fmus-embed library
 *
 * This header provides comprehensive motor control functionality including
 * DC motors, servo motors, and stepper motors with advanced features like
 * PWM control, direction control, and safety mechanisms.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace fmus {
namespace actuators {

/**
 * @brief Motor types enumeration
 */
enum class MotorType : uint8_t {
    DC = 0,        ///< DC motor
    Servo = 1,     ///< Servo motor
    Stepper = 2    ///< Stepper motor
};

/**
 * @brief Motor direction enumeration
 */
enum class MotorDirection : uint8_t {
    Forward = 0,   ///< Forward direction
    Reverse = 1,   ///< Reverse direction
    Brake = 2,     ///< Brake/stop
    Coast = 3      ///< Coast/free run
};

/**
 * @brief Stepper motor step modes
 */
enum class StepMode : uint8_t {
    Full = 1,      ///< Full step mode
    Half = 2,      ///< Half step mode
    Quarter = 4,   ///< Quarter step mode
    Eighth = 8,    ///< Eighth step mode
    Sixteenth = 16 ///< Sixteenth step mode
};

/**
 * @brief Base motor interface
 */
class FMUS_EMBED_API IMotor {
public:
    virtual ~IMotor() = default;

    /**
     * @brief Initialize the motor
     *
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> init() = 0;

    /**
     * @brief Check if the motor is initialized
     *
     * @return bool True if initialized
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Stop the motor
     *
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> stop() = 0;

    /**
     * @brief Get the motor type
     *
     * @return MotorType The motor type
     */
    virtual MotorType getType() const = 0;

    /**
     * @brief Get motor status information
     *
     * @return std::string Status information
     */
    virtual std::string getStatus() const = 0;
};

/**
 * @brief DC Motor control class
 */
class FMUS_EMBED_API DCMotor : public IMotor {
public:
    /**
     * @brief Construct a new DC Motor
     *
     * @param pwmPin PWM control pin
     * @param directionPin Direction control pin (optional, use 255 for single direction)
     * @param enablePin Enable pin (optional, use 255 if not used)
     */
    DCMotor(uint8_t pwmPin, uint8_t directionPin = 255, uint8_t enablePin = 255);

    /**
     * @brief Destructor
     */
    ~DCMotor() override;

    // IMotor interface
    core::Result<void> init() override;
    bool isInitialized() const override;
    core::Result<void> stop() override;
    MotorType getType() const override;
    std::string getStatus() const override;

    /**
     * @brief Set motor speed
     *
     * @param speed Speed value (0.0 to 1.0)
     * @return core::Result<void> Success or error
     */
    core::Result<void> setSpeed(float speed);

    /**
     * @brief Set motor direction
     *
     * @param direction Motor direction
     * @return core::Result<void> Success or error
     */
    core::Result<void> setDirection(MotorDirection direction);

    /**
     * @brief Set speed and direction in one call
     *
     * @param speed Speed value (-1.0 to 1.0, negative for reverse)
     * @return core::Result<void> Success or error
     */
    core::Result<void> setSpeedAndDirection(float speed);

    /**
     * @brief Enable/disable motor
     *
     * @param enabled True to enable, false to disable
     * @return core::Result<void> Success or error
     */
    core::Result<void> setEnabled(bool enabled);

    /**
     * @brief Get current speed
     *
     * @return float Current speed (0.0 to 1.0)
     */
    float getSpeed() const;

    /**
     * @brief Get current direction
     *
     * @return MotorDirection Current direction
     */
    MotorDirection getDirection() const;

    /**
     * @brief Check if motor is enabled
     *
     * @return bool True if enabled
     */
    bool isEnabled() const;

    /**
     * @brief Set PWM frequency
     *
     * @param frequency PWM frequency in Hz
     * @return core::Result<void> Success or error
     */
    core::Result<void> setPWMFrequency(uint32_t frequency);

private:
    uint8_t m_pwmPin;           ///< PWM control pin
    uint8_t m_directionPin;     ///< Direction control pin
    uint8_t m_enablePin;        ///< Enable control pin
    bool m_initialized;         ///< Initialization state
    float m_speed;              ///< Current speed (0.0 to 1.0)
    MotorDirection m_direction; ///< Current direction
    bool m_enabled;             ///< Enable state
    uint32_t m_pwmFrequency;    ///< PWM frequency
};

/**
 * @brief Servo Motor control class
 */
class FMUS_EMBED_API ServoMotor : public IMotor {
public:
    /**
     * @brief Construct a new Servo Motor
     *
     * @param pwmPin PWM control pin
     * @param minPulseWidth Minimum pulse width in microseconds (default: 1000)
     * @param maxPulseWidth Maximum pulse width in microseconds (default: 2000)
     */
    ServoMotor(uint8_t pwmPin, uint16_t minPulseWidth = 1000, uint16_t maxPulseWidth = 2000);

    /**
     * @brief Destructor
     */
    ~ServoMotor() override;

    // IMotor interface
    core::Result<void> init() override;
    bool isInitialized() const override;
    core::Result<void> stop() override;
    MotorType getType() const override;
    std::string getStatus() const override;

    /**
     * @brief Set servo angle
     *
     * @param angle Angle in degrees (0 to 180)
     * @return core::Result<void> Success or error
     */
    core::Result<void> setAngle(float angle);

    /**
     * @brief Get current angle
     *
     * @return float Current angle in degrees
     */
    float getAngle() const;

    /**
     * @brief Sweep between two angles
     *
     * @param startAngle Start angle in degrees
     * @param endAngle End angle in degrees
     * @param duration Duration in milliseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> sweep(float startAngle, float endAngle, uint32_t duration);

    /**
     * @brief Set pulse width directly
     *
     * @param pulseWidth Pulse width in microseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> setPulseWidth(uint16_t pulseWidth);

private:
    uint8_t m_pwmPin;           ///< PWM control pin
    uint16_t m_minPulseWidth;   ///< Minimum pulse width (µs)
    uint16_t m_maxPulseWidth;   ///< Maximum pulse width (µs)
    bool m_initialized;         ///< Initialization state
    float m_currentAngle;       ///< Current angle
    uint16_t m_currentPulseWidth; ///< Current pulse width
};

/**
 * @brief Stepper Motor control class
 */
class FMUS_EMBED_API StepperMotor : public IMotor {
public:
    /**
     * @brief Construct a new Stepper Motor (4-wire)
     *
     * @param pin1 Coil 1 pin
     * @param pin2 Coil 2 pin
     * @param pin3 Coil 3 pin
     * @param pin4 Coil 4 pin
     * @param stepsPerRevolution Steps per full revolution (default: 200)
     */
    StepperMotor(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, 
                 uint16_t stepsPerRevolution = 200);

    /**
     * @brief Destructor
     */
    ~StepperMotor() override;

    // IMotor interface
    core::Result<void> init() override;
    bool isInitialized() const override;
    core::Result<void> stop() override;
    MotorType getType() const override;
    std::string getStatus() const override;

    /**
     * @brief Step the motor
     *
     * @param steps Number of steps (positive for forward, negative for reverse)
     * @return core::Result<void> Success or error
     */
    core::Result<void> step(int32_t steps);

    /**
     * @brief Set step mode
     *
     * @param mode Step mode
     * @return core::Result<void> Success or error
     */
    core::Result<void> setStepMode(StepMode mode);

    /**
     * @brief Set step delay
     *
     * @param delayMicroseconds Delay between steps in microseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> setStepDelay(uint32_t delayMicroseconds);

    /**
     * @brief Rotate by angle
     *
     * @param degrees Angle in degrees
     * @return core::Result<void> Success or error
     */
    core::Result<void> rotate(float degrees);

    /**
     * @brief Get current position
     *
     * @return int32_t Current position in steps
     */
    int32_t getPosition() const;

    /**
     * @brief Reset position to zero
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> resetPosition();

private:
    uint8_t m_pins[4];              ///< Control pins
    uint16_t m_stepsPerRevolution;  ///< Steps per revolution
    bool m_initialized;             ///< Initialization state
    int32_t m_currentPosition;      ///< Current position in steps
    StepMode m_stepMode;            ///< Current step mode
    uint32_t m_stepDelay;           ///< Delay between steps (µs)
    uint8_t m_currentStep;          ///< Current step in sequence

    /**
     * @brief Execute a single step
     *
     * @param direction 1 for forward, -1 for reverse
     */
    void executeStep(int8_t direction);
};

/**
 * @brief Get string representation of motor type
 *
 * @param type Motor type
 * @return std::string String representation
 */
FMUS_EMBED_API std::string motorTypeToString(MotorType type);

/**
 * @brief Get string representation of motor direction
 *
 * @param direction Motor direction
 * @return std::string String representation
 */
FMUS_EMBED_API std::string motorDirectionToString(MotorDirection direction);

/**
 * @brief Get string representation of step mode
 *
 * @param mode Step mode
 * @return std::string String representation
 */
FMUS_EMBED_API std::string stepModeToString(StepMode mode);

} // namespace actuators
} // namespace fmus
