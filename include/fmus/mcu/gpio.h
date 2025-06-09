#pragma once

/**
 * @file gpio.h
 * @brief MCU-specific GPIO interface for the fmus-embed library
 *
 * This header provides MCU-specific GPIO functionality that abstracts
 * hardware-specific implementations.
 */

#include "../fmus_config.h"
#include "../core/result.h"

namespace fmus {
namespace mcu {

/**
 * @brief GPIO pin modes
 */
enum class GpioMode {
    Input,          ///< Input pin mode
    Output,         ///< Output pin mode
    InputPullUp,    ///< Input with internal pull-up resistor
    InputPullDown,  ///< Input with internal pull-down resistor
    AnalogInput,    ///< Analog input mode
    AnalogOutput,   ///< Analog output mode (PWM)
    AlternateFunc,  ///< Alternate function mode
};

/**
 * @brief GPIO pin states
 */
enum class GpioState {
    Low,    ///< Logic low state
    High,   ///< Logic high state
};

/**
 * @brief Initialize the GPIO subsystem
 *
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> initGpio();

/**
 * @brief Configure a GPIO pin
 *
 * @param pin Pin number
 * @param mode Pin mode
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> configurePin(uint8_t pin, GpioMode mode);

/**
 * @brief Set the state of a GPIO pin
 *
 * @param pin Pin number
 * @param state Pin state
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> writePin(uint8_t pin, GpioState state);

/**
 * @brief Read the state of a GPIO pin
 *
 * @param pin Pin number
 * @return Result containing the pin state or an error
 */
FMUS_EMBED_API core::Result<GpioState> readPin(uint8_t pin);

/**
 * @brief Write an analog value to a GPIO pin (PWM)
 *
 * @param pin Pin number
 * @param value Analog value (0-255)
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> writeAnalog(uint8_t pin, uint8_t value);

/**
 * @brief Read an analog value from a GPIO pin
 *
 * @param pin Pin number
 * @return Result containing the analog value (0-1023) or an error
 */
FMUS_EMBED_API core::Result<uint16_t> readAnalog(uint8_t pin);

} // namespace mcu
} // namespace fmus
