#pragma once

/**
 * @file actuators.h
 * @brief Main actuators header for the fmus-embed library
 *
 * This header includes all actuator control functionality including
 * motors, servos, relays, and other output devices.
 */

#include "motor.h"
#include "servo.h"
#include "relay.h"

namespace fmus {
namespace actuators {

/**
 * @brief Initialize the actuators module
 *
 * @return core::Result<void> Success or error
 */
FMUS_EMBED_API core::Result<void> initActuators();

/**
 * @brief Shutdown the actuators module
 *
 * @return core::Result<void> Success or error
 */
FMUS_EMBED_API core::Result<void> shutdownActuators();

/**
 * @brief Check if actuators module is initialized
 *
 * @return bool True if initialized
 */
FMUS_EMBED_API bool isActuatorsInitialized();

/**
 * @brief Emergency stop all actuators
 *
 * This function immediately stops all motors, servos, and turns off all relays
 * for safety purposes.
 *
 * @return core::Result<void> Success or error
 */
FMUS_EMBED_API core::Result<void> emergencyStopAll();

/**
 * @brief Get actuators module status
 *
 * @return std::string Status information
 */
FMUS_EMBED_API std::string getActuatorsStatus();

} // namespace actuators
} // namespace fmus
