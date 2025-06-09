#pragma once

/**
 * @file timer.h
 * @brief MCU-specific timer interface for the fmus-embed library
 *
 * This header provides MCU-specific timer functionality that abstracts
 * hardware-specific implementations.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <functional>

namespace fmus {
namespace mcu {

/**
 * @brief Timer modes
 */
enum class TimerMode {
    OneShot,    ///< Timer runs once and stops
    Periodic,   ///< Timer runs repeatedly
};

/**
 * @brief Timer callback function type
 */
using TimerCallback = std::function<void()>;

/**
 * @brief Timer handle type
 */
using TimerHandle = uint32_t;

/**
 * @brief Initialize the timer subsystem
 *
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> initTimers();

/**
 * @brief Create a new timer
 *
 * @param callback Function to call when timer expires
 * @param intervalMs Timer interval in milliseconds
 * @param mode Timer mode (one-shot or periodic)
 * @return Result containing the timer handle or an error
 */
FMUS_EMBED_API core::Result<TimerHandle> createTimer(TimerCallback callback, uint32_t intervalMs, TimerMode mode);

/**
 * @brief Start a timer
 *
 * @param handle Timer handle
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> startTimer(TimerHandle handle);

/**
 * @brief Stop a timer
 *
 * @param handle Timer handle
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> stopTimer(TimerHandle handle);

/**
 * @brief Reset a timer
 *
 * @param handle Timer handle
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> resetTimer(TimerHandle handle);

/**
 * @brief Delete a timer and free resources
 *
 * @param handle Timer handle
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> deleteTimer(TimerHandle handle);

/**
 * @brief Get the current system time in milliseconds
 *
 * @return Current system time in milliseconds
 */
FMUS_EMBED_API uint32_t getTimeMs();

/**
 * @brief Delay execution for a specified number of milliseconds
 *
 * @param ms Milliseconds to delay
 */
FMUS_EMBED_API void delayMs(uint32_t ms);

/**
 * @brief Delay execution for a specified number of microseconds
 *
 * @param us Microseconds to delay
 */
FMUS_EMBED_API void delayUs(uint32_t us);

} // namespace mcu
} // namespace fmus
