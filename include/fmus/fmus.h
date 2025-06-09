#pragma once

/**
 * @file fmus.h
 * @brief Main header for the fmus-embed library
 *
 * This header includes all the components of the fmus-embed library.
 */

// Configuration
#include "fmus_config.h"

// Core components
#include "core/error.h"
#include "core/logging.h"
#include "core/memory.h"
#include "core/result.h"
#include "core/version.h"

// MCU module
#include "mcu/gpio.h"
#include "mcu/timer.h"
#include "mcu/adc.h"
#include "mcu/platform.h"

// GPIO module
#include "gpio/gpio.h"

// Sensors module
#include "sensors/sensor.h"
#include "sensors/accelerometer.h"
#include "sensors/temperature.h"

// Actuators module
// TODO: Implement these files
// #include "actuators/motor.h"
// #include "actuators/relay.h"
// #include "actuators/servo.h"

// Communications module
#include "comms/spi.h"
#include "comms/i2c.h"
// #include "comms/uart.h"

// DSP module
// #include "dsp/filter.h"
// #include "dsp/fft.h"

// AI module
// #include "ai/neural_net.h"

// Networking module
// #include "net/mqtt.h"

namespace fmus {

/**
 * @brief Initialize the fmus-embed library
 *
 * This function initializes the library and its components.
 *
 * @return True if initialization was successful, false otherwise
 */
FMUS_EMBED_API bool init();

/**
 * @brief Shutdown the fmus-embed library
 *
 * This function releases resources and shuts down the library components.
 */
FMUS_EMBED_API void shutdown();

} // namespace fmus
