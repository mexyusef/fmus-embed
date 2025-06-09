#include "fmus/fmus.h"
#include "fmus/core/logging.h"
#include "fmus/mcu/platform.h"
#include "fmus/mcu/gpio.h"
#include "fmus/mcu/timer.h"
#include "fmus/mcu/adc.h"
#include "fmus/comms/i2c.h"
#include "fmus/comms/spi.h"

namespace fmus {

bool init() {
    FMUS_LOG_INFO("Initializing fmus-embed library");

    // Initialize platform
    auto platformResult = mcu::initPlatform();
    if (platformResult.isError()) {
        FMUS_LOG_ERROR("Failed to initialize platform: " + platformResult.error().message());
        return false;
    }

    // Initialize GPIO
    auto gpioResult = mcu::initGpio();
    if (gpioResult.isError()) {
        FMUS_LOG_ERROR("Failed to initialize GPIO: " + gpioResult.error().message());
        return false;
    }

    // Initialize timers
    auto timerResult = mcu::initTimers();
    if (timerResult.isError()) {
        FMUS_LOG_ERROR("Failed to initialize timers: " + timerResult.error().message());
        return false;
    }

    // Initialize ADC
    auto adcResult = mcu::initAdc();
    if (adcResult.isError()) {
        FMUS_LOG_ERROR("Failed to initialize ADC: " + adcResult.error().message());
        return false;
    }

    FMUS_LOG_INFO("fmus-embed library initialized successfully");
    return true;
}

void shutdown() {
    FMUS_LOG_INFO("Shutting down fmus-embed library");

    // Perform cleanup operations here

    FMUS_LOG_INFO("fmus-embed library shut down successfully");
}

} // namespace fmus
