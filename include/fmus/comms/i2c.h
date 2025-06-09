#pragma once

/**
 * @file i2c.h
 * @brief I2C communication interface for the fmus-embed library
 *
 * This header provides I2C (Inter-Integrated Circuit) communication functionality.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <vector>
#include <cstdint>

namespace fmus {
namespace comms {

/**
 * @brief I2C bus speeds
 */
enum class I2cSpeed {
    Standard = 100000,  ///< Standard mode: 100 kHz
    Fast = 400000,      ///< Fast mode: 400 kHz
    FastPlus = 1000000, ///< Fast mode plus: 1 MHz
    HighSpeed = 3400000 ///< High speed mode: 3.4 MHz
};

/**
 * @brief I2C configuration structure
 */
struct I2cConfig {
    uint8_t busNumber;       ///< I2C bus number
    I2cSpeed speed;          ///< I2C bus speed
    bool pullUpsEnabled;     ///< Whether to enable internal pull-up resistors
    uint16_t timeoutMs;      ///< Timeout in milliseconds

    /**
     * @brief Constructor with default values
     */
    I2cConfig(uint8_t bus = 0,
              I2cSpeed spd = I2cSpeed::Standard,
              bool pullUps = true,
              uint16_t timeout = 1000)
        : busNumber(bus), speed(spd), pullUpsEnabled(pullUps), timeoutMs(timeout) {}
};

/**
 * @brief I2C master device class
 *
 * This class provides I2C master functionality for communicating with I2C slave devices.
 */
class I2cMaster {
public:
    /**
     * @brief Constructor
     *
     * @param config I2C configuration
     */
    FMUS_EMBED_API I2cMaster(const I2cConfig& config = I2cConfig());

    /**
     * @brief Destructor
     */
    FMUS_EMBED_API ~I2cMaster();

    /**
     * @brief Initialize the I2C bus
     *
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> init();

    /**
     * @brief Check if a device is present at the specified address
     *
     * @param deviceAddress 7-bit I2C device address
     * @return Result indicating whether the device is present
     */
    FMUS_EMBED_API core::Result<bool> ping(uint8_t deviceAddress);

    /**
     * @brief Write data to an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param data Data to write
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> write(uint8_t deviceAddress, const std::vector<uint8_t>& data);

    /**
     * @brief Read data from an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param length Number of bytes to read
     * @return Result containing the read data or an error
     */
    FMUS_EMBED_API core::Result<std::vector<uint8_t>> read(uint8_t deviceAddress, size_t length);

    /**
     * @brief Write data to a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @param data Data to write
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> writeRegister(uint8_t deviceAddress, uint8_t regAddress, const std::vector<uint8_t>& data);

    /**
     * @brief Read data from a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @param length Number of bytes to read
     * @return Result containing the read data or an error
     */
    FMUS_EMBED_API core::Result<std::vector<uint8_t>> readRegister(uint8_t deviceAddress, uint8_t regAddress, size_t length);

    /**
     * @brief Write a single byte to a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @param value Byte to write
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> writeRegisterByte(uint8_t deviceAddress, uint8_t regAddress, uint8_t value);

    /**
     * @brief Read a single byte from a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @return Result containing the read byte or an error
     */
    FMUS_EMBED_API core::Result<uint8_t> readRegisterByte(uint8_t deviceAddress, uint8_t regAddress);

    /**
     * @brief Write a 16-bit value to a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @param value 16-bit value to write
     * @param bigEndian Whether to use big-endian byte order (default: true)
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> writeRegisterWord(uint8_t deviceAddress, uint8_t regAddress, uint16_t value, bool bigEndian = true);

    /**
     * @brief Read a 16-bit value from a specific register in an I2C device
     *
     * @param deviceAddress 7-bit I2C device address
     * @param regAddress Register address
     * @param bigEndian Whether to use big-endian byte order (default: true)
     * @return Result containing the read 16-bit value or an error
     */
    FMUS_EMBED_API core::Result<uint16_t> readRegisterWord(uint8_t deviceAddress, uint8_t regAddress, bool bigEndian = true);

    /**
     * @brief Set the I2C bus speed
     *
     * @param speed I2C bus speed
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> setSpeed(I2cSpeed speed);

    /**
     * @brief Set the I2C bus timeout
     *
     * @param timeoutMs Timeout in milliseconds
     * @return Result indicating success or failure
     */
    FMUS_EMBED_API core::Result<void> setTimeout(uint16_t timeoutMs);

    /**
     * @brief Get the current I2C configuration
     *
     * @return Current I2C configuration
     */
    FMUS_EMBED_API const I2cConfig& getConfig() const;

private:
    I2cConfig m_config;
    bool m_initialized;
    void* m_handle; // Platform-specific handle
};

/**
 * @brief Scan the I2C bus for devices
 *
 * @param i2c I2C master instance
 * @return Result containing a vector of detected device addresses or an error
 */
FMUS_EMBED_API core::Result<std::vector<uint8_t>> scanI2cBus(I2cMaster& i2c);

} // namespace comms
} // namespace fmus
