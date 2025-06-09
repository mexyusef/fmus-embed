#pragma once

/**
 * @file platform.h
 * @brief MCU-specific platform interface for the fmus-embed library
 *
 * This header provides MCU-specific platform functionality that abstracts
 * hardware-specific implementations.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <string>

namespace fmus {
namespace mcu {

/**
 * @brief MCU platform types
 */
enum class PlatformType {
    Unknown,        ///< Unknown platform
    AVR,            ///< AVR-based platforms (Arduino Uno, Mega, etc.)
    ARM_Cortex_M0,  ///< ARM Cortex-M0 platforms
    ARM_Cortex_M3,  ///< ARM Cortex-M3 platforms
    ARM_Cortex_M4,  ///< ARM Cortex-M4 platforms
    ARM_Cortex_M7,  ///< ARM Cortex-M7 platforms
    ESP8266,        ///< ESP8266 platforms
    ESP32,          ///< ESP32 platforms
    STM32,          ///< STM32 platforms
    Simulator       ///< Simulator platform
};

/**
 * @brief Platform information structure
 */
struct PlatformInfo {
    PlatformType type;       ///< Platform type
    std::string name;        ///< Platform name
    std::string mcuModel;    ///< MCU model
    uint32_t clockSpeedHz;   ///< CPU clock speed in Hz
    uint32_t flashSizeKB;    ///< Flash memory size in KB
    uint32_t ramSizeKB;      ///< RAM size in KB
    uint8_t cpuCores;        ///< Number of CPU cores
    uint32_t cpuFreqMHz;     ///< CPU frequency in MHz
    std::string version;     ///< Platform version
};

/**
 * @brief Initialize the platform subsystem
 *
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> initPlatform();

/**
 * @brief Get platform information
 *
 * @return Platform information structure
 */
FMUS_EMBED_API PlatformInfo getPlatformInfo();

/**
 * @brief Get the CPU temperature
 *
 * @return Result containing the CPU temperature in Celsius or an error
 */
FMUS_EMBED_API core::Result<float> getCpuTemperature();

/**
 * @brief Get the CPU usage percentage
 *
 * @return Result containing the CPU usage percentage or an error
 */
FMUS_EMBED_API core::Result<float> getCpuUsage();

/**
 * @brief Get the free RAM in bytes
 *
 * @return Result containing the amount of free RAM in bytes or an error
 */
FMUS_EMBED_API core::Result<uint32_t> getFreeRam();

/**
 * @brief Get the system uptime in milliseconds
 *
 * @return System uptime in milliseconds
 */
FMUS_EMBED_API uint32_t getUptime();

/**
 * @brief Restart the MCU
 *
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> restart();

/**
 * @brief Enter deep sleep mode for a specified time
 *
 * @param ms Time to sleep in milliseconds
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> deepSleep(uint32_t ms);

/**
 * @brief Get the unique device ID
 *
 * @return Result containing the unique device ID or an error
 */
FMUS_EMBED_API core::Result<std::string> getDeviceId();

/**
 * @brief Get the platform name
 *
 * @return Result containing the platform name or an error
 */
FMUS_EMBED_API core::Result<std::string> getPlatformName();

/**
 * @brief Get the version string
 *
 * @return Result containing the version information or an error
 */
FMUS_EMBED_API core::Result<std::string> getVersionString();

} // namespace mcu
} // namespace fmus
